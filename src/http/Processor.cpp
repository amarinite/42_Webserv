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
	: _req(req), _res(res), _lc(lc) {}

// Processor &Processor::operator=(const Processor &p) {
// 	if (this != &a) {
// 		_req = p._req;
// 		_lc = p._lc;
// 	}
// }

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

/**
 * @brief Creates a file and fill it with the body parsed in the Http Request.
 * 
 * @throws HttpException 500 if ti fails creating th file.
 */
void Processor::createFile() {
	std::ofstream newFile(_fullPath.c_str());
	if (newFile.is_open()) {
		newFile << _req.getBody();
		newFile.close();
	} else
		throw HttpException(500, "Internal Server Error: error creating file.");
}

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
		throw HttpException(403, "Forbidden");

	DIR *folder = opendir(_fullPath.c_str());
	if (folder == NULL) {
		if (errno == EACCES)
			throw HttpException(403, "Forbidden");
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
	
}

/**
 * @brief Unifies de functions of GET method and sets the Status code and message.
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

// /**
//  * @brief Unifies de functions of POST method and sets the Status code and message.
//  */
// void Processor::handlePost() {
// 	validateDirectory();
// 	createFile();
// 	_code = 201;
// 	_codeMsg = "Created";
// }

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

// Routine
/**
 * @brief Derives the processing of the request to a handler depending on the Method.
 * 
 * @param method Method extracted in the Parse of the Http Request. 
 */
void Processor::processorRoutine() {
	_fullPath = concatPaths(_lc.getRoot(), _req.getPath());
	if (!isValidMethod()) {
		throw HttpException(405, "Method Not Allowed", findAllowedMethods(_lc.getAllowedMethods()));
	}
	// Redirect
	// CGI
	if (_req.getMethod() == "GET")
		handleGet();
	// else if (_req.getMethod() ==  "POST")
		// handlePost();
	else if (_req.getMethod() ==  "DELETE")
		handleDelete();
	else
	   	throw HttpException(501, "Not Implemented");
}

/**
 * @brief assigns headers and builds Http Response.
 * 
 */
void Processor::prepareResponse() {
	if (!_res.getResponseBody().empty())
		_res.setResponseBody(_responseBody);
	_res.assignHeaders(_extension, _req.getConnection());
	if (_code == "301")
		_res.setLocationHeader(_lc.getRedirect().path);
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
