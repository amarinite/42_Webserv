#include "Processor.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationConfig.hpp"
#include "FileUtils.hpp"

#include <sys/stat.h>
#include <fstream>
#include <dirent.h>

/**
 * @brief Construct a new Processor:: Processor object
 *
 * @param req Http Request Info
 * @param lc LocationConfig info
 * @param res Empty Response to store results.
 */
Processor::Processor(Request &req, Response &res, const LocationConfig &lc)
	: _cgiRequested(false), _lc(lc), _req(req), _res(res) {}

/**
 * @brief Concatenates root directory with the requested directory
 *
 * @param root Base directory of the Processor.
 * @param path New directory extracted from the HTTP request.
 * @returns std::stirng with concatenated paths.
 */
static std::string concatPaths(const std::string &root, const std::string &path) {
	if (root.empty())
		return path;
	if (path.empty())
		return root;

	bool rootHasSlash = (root[root.length() - 1] == '/');
	bool pathHasSlash = (path[0] == '/');

	if (!rootHasSlash && !pathHasSlash)
		return root + "/" + path;

	if (rootHasSlash && pathHasSlash)
		return root + path.substr(1);
	return root + path;
}

/**
 * @brief Checks if the extension is valid.
 *
 * @param ext extension to verify.
 */
void Processor::convertFileExtension(const std::string &ext) {
	MimeTypes map;
	_extension = map.getType(ext);
}

// For POST
// void Processor::handleMultipart(std::string &content) {
// 	size_t pos = content.find("boundary")
// 	if (pos == content.end())
// 		throw HttpException(400, "Bad Request: Bad Header");
// 	std::string boundary = "--" + content.substr(10);
// }

// void Processor::handleContentType() {
// 	std::map<std::string, std::string>::iterator it = _req._headers.find("content-type");
// 	if (it == _req._headers.end())
// 		throw HttpException(400, "Bad Request: Missing Content-Type header");
// 	if (it->second.find("multipart/form-data"))
// 		handleMultipart(it->second);
// 	else if (it->second.find("application/x-www-form-urlencoded"))
// 		handleXForm(it->second);
// 	else if (it->second.find("text/plain") || it->second.find("application/octet-stream"))
// 		handlePlainTxt(it->second);
// }

// /**
//  * @brief Creates a file and fill it with the body parsed in the Http Request.
//  *
//  * @throws HttpException 500 if ti fails creating th file.
//  */
// void Processor::createFile() {
// 	std::ofstream newFile(_fullPath.c_str());
// 	if (newFile.is_open()) {
// 		newFile << _req.getBody();
// 		newFile.close();
// 	} else
// 		throw HttpException(500, "Internal Server Error: error creating file.");
// }

/**
 * @brief checks for multiple index pages and returns de first that exists.
 *
 * @return true If a valid index file is found.
 * @return false If no configured index exists or is inaccesible.
 */
bool Processor::findIndexPage() {
	const std::vector<std::string> &indexes = _lc.getIndex();
	std::vector<std::string>::const_iterator it = indexes.begin();
	for (; it != indexes.end(); ++it) {
		std::string potentialIdx = concatPaths(_fullPath, *it);
		std::cout << potentialIdx << std::endl;
		try {
			if (validateFile(potentialIdx)) {
				_fullPath = potentialIdx;
				return true;
			}
		} catch (const HttpException &e) {
			continue;
		}
	}
	return false;
}

/**
 * @brief Validates directory permissions.
 *
 * @return const std::string validated path.
 */
const std::string Processor::requestPath() const {
	validateDir(_req.getPath());
	return _req.getPath();
}

/**
 * @brief Creates the autoindex page.
 *
 * @throws HttpException 403 if user has no permits.
 * @throws HttpException 404 if directory doesnt exist.
 * @throws HttpException 500 if error of opendir.
 */
void Processor::doAutoIndex() {
	if (!_lc.hasAutoIndex())
		throw HttpException(403, "Forbidden 1");

	DIR *folder = opendir(_fullPath.c_str());
	if (folder == NULL) {
		if (errno == EACCES)
			throw HttpException(403, "Forbidden 2");
		else if (errno == ENOENT)
			throw HttpException(404, "Not Found");
		else
			throw HttpException(500, "Internal Server Error");
	}

	std::string path = requestPath();
	std::stringstream html;

	html << "<html>\n<head><title>Index of " << path << "</title></head>\n";
	html << "<body style=\"font-family: sans-serif; padding: 20px;\">\n";
	html << "<h1>Index of " << path << "</h1>\n<hr>\n<ul>\n";

	struct dirent *content;
	while ((content = readdir(folder)) != NULL)
		html << "<li>" << content->d_name << "</li>\n";

	closedir(folder);

	html << "</ul>\n<hr>\n</body>\n</html>";

	_responseBody = html.str();
	_extension = ".html";

}

/**
 * @brief Unifies the functions of GET method and sets the Status code and message.
 */
void Processor::handleGet() {
	bool isDir = validatePathDir(_fullPath);
	if (!isDir) {
			_extension = findFileExtension(_fullPath);
			_responseBody = readFile(_fullPath);
	}
	else {
		if (findIndexPage()) {
			_extension = findFileExtension(_fullPath);
			_responseBody = readFile(_fullPath);
		} else
			doAutoIndex();
	}
	_code = "200";
	_codeMsg = "Ok";
}

/**
 * @brief Unifies the functions of POST method and sets the Status code and message.
 */
void Processor::handlePost() {
	if (!_lc.hasUploadEnabled())
		throw HttpException(403, "Forbidden: Upload not allowed");

	const std::string uploadPath = _lc.getUploadStore();
	validateDir(uploadPath);
	if (access(uploadPath.c_str(), W_OK) != 0)
		throw HttpException(403, "Forbidden: Upload directory is not writable");

	std::string uriPath = _req.getPath();
	if (uriPath[uriPath.size() - 1] == '/')
		uriPath.erase(uriPath.size() - 1);

	std::string filename;
	size_t lastSlash = uriPath.rfind('/');
	if (lastSlash != std::string::npos)
		filename = uriPath.substr(lastSlash + 1);
	else
		filename = uriPath;

	if (filename.empty() || filename.find("..") != std::string::npos)
		throw HttpException(400, "Bad Request: Invalid filename");

	_fullPath = concatPaths(uploadPath, filename);
	createFile(_fullPath, _req.getBody());
	_code = "201";
	_codeMsg = "Created";

}

/**
 * @brief Unifies de functions of DELETE method and sets the Status code and message.
 */
void Processor::handleDelete() {
	validateFile(_fullPath);
	removeFile(_fullPath);
	_code = "204";
	_codeMsg = "No Content";
	_responseBody.clear();
}

bool Processor::isValidMethod() {
	const std::vector<std::string> &methods = _lc.getAllowedMethods();
	std::vector<std::string>::const_iterator it = methods.begin();
	for (; it != methods.end(); ++it) {
		if (*it == _req.getMethod())
			return true;
	}
	return false;
}

static std::string findAllowedMethods(const std::vector<std::string> &allowed) {
	std::ostringstream oss;
	for (size_t i = 0; i < allowed.size(); ++i) {
		if (i != 0)
			oss << ", ";
		oss << allowed[i];
	}
	return oss.str();
}

bool Processor::isRedirect() const {
	return !_lc.getRedirect().path.empty();
}

void Processor::handleRedirect() {
	const t_uri &redirect = _lc.getRedirect();

	_code = "301";
	_codeMsg = "Moved Permanently";
	_redirectPath = toString(redirect);
	_responseBody.clear();
	// if browser shows error
	// _responseBody = "<html><body><a href=\"" + _redirectPath + "\">Redirecting...</a></body></html>";
}

// Routine
/**
 * @brief Derives the processing of the request to a handler depending on the Method.
 *
 * @param method Method extracted in the Parse of the Http Request.
 */
void Processor::processorRoutine() {
	if (isRedirect()) {
		handleRedirect();
		return;
	}

	_fullPath = concatPaths(_lc.getRoot(), _req.getPath());
	if (!isValidMethod()) {
		throw HttpException(405, "Method Not Allowed", findAllowedMethods(_lc.getAllowedMethods()));
	}
	// CGI
	if (CgiHandler::canHandleCgi(_req.getUri(), _lc)) {
		prepareCgi();
		return; // Http will see wantsCgi() == true and start CgiExecutor itself
	}
	if (_req.getMethod() == "GET")
		handleGet();
	else if (_req.getMethod() ==  "POST")
		handlePost();
	else if (_req.getMethod() ==  "DELETE")
		handleDelete();
	else
		throw HttpException(501, "Not Implemented");
}

void Processor::prepareCgi() {
	_cgiScriptPath = _fullPath;
	std::string ext = findFileExtension(_cgiScriptPath);
	const std::map<std::string, std::string> &cgiExt = _lc.getCgiExtension();
	std::map<std::string, std::string>::const_iterator it = cgiExt.find(ext);
	if (it == cgiExt.end())
		throw HttpException(500, "Internal Server Error: CGI misconfiguration");
	_cgiExecPath = it->second;
	_cgiRequested = true;
}

bool Processor::wantsCgi() const {
	return _cgiRequested;
}

const std::string &Processor::getCgiScriptPath() const {
	return _cgiScriptPath;
}

const std::string &Processor::getCgiExecPath() const {
	return _cgiExecPath;
}

void Processor::consumeCgiOutput(const std::string &rawCgiOutput) {
	size_t sepLen = 4;
	size_t headerEnd = rawCgiOutput.find("\r\n\r\n");
	if (headerEnd == std::string::npos) {
		sepLen = 2;
		headerEnd = rawCgiOutput.find("\n\n");
	}
	if (headerEnd == std::string::npos)
		throw HttpException(502, "Bad Gateway: Malformed CGI Output");

	std::string headerBlock = rawCgiOutput.substr(0, headerEnd);
	_responseBody = rawCgiOutput.substr(headerEnd + sepLen);

	_code = "200";
	_codeMsg = "OK";

	std::istringstream stream(headerBlock);
	std::string line;
	while (std::getline(stream, line)) {
		if (!line.empty() && line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		if (line.empty())
			continue;
		size_t colon = line.find(':');
		if (colon == std::string::npos)
			continue;
		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		size_t firstNonSpace = value.find_first_not_of(' ');
		if (firstNonSpace != std::string::npos)
			value = value.substr(firstNonSpace);

		if (key == "Status") {
			_code = value.substr(0, 3);
			_codeMsg = value.size() > 4 ? value.substr(4) : "";
		} else if (key == "Location") {
			_redirectPath = value;
			if (_code == "200") { _code = "302"; _codeMsg = "Found"; }
		} else {
			_res.addRawHeader(key, value);
		}
	}
}

/**
 * @brief assigns headers and builds Http Response.
 *
 */
void Processor::prepareResponse() {
	_res.setStatusCode(_code);
	_res.setMessage(_codeMsg);
	if (!_responseBody.empty())
		_res.setResponseBody(_responseBody);

	if (_cgiRequested)
		_res.assignConnectionAndLengthHeaders(_req.getConnection());
	else
		_res.assignHeaders(_extension, _req.getConnection());

	if (_code == "301" || _code == "302")
		_res.setLocationHeader(_redirectPath);

	_res.buildRawResponse();
}

// Getters.
/**
 * @brief Getter to full path variable saved in the object.
 *
 * @return std::string The fullPath variable.
 */
const std::string &Processor::getFullPath() const {
	return _fullPath;
}

/**
 * @brief Getter to the extension variable saved in the object.
 *
 * @return std::string The Extension variable.
 */
const std::string &Processor::getExtension() const {
	return _extension;
}

/**
 * @brief Getter to the body variable saved in the object.
 *
 * @return std::string The Response Body variable.
 */
const std::string &Processor::getResponseBody() const {
	return _responseBody;
}

/**
 * @brief Getter to the status code variable saved in the object.
 *
 * @return std::string The Status Code variable.
 */
const std::string &Processor::getStatusCode() const {
	return _code;
}
