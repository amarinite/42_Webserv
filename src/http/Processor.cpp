#include "Processor.hpp"

// /**
//  * @brief Construct a new empty Processor:: Processor object
//  */
// // Processor::Processor() {}


// Processor::Processor(const Processor &p) {
// 	this = &p;
// }

/**
 * @brief Construct a new Processor:: Processor object
 * 
 * @param req Http Request Info
 * @param lc LocationConfig info
 * @param res Empty Response to store results.
 */
Processor::Processor(Request &req, LocationConfig &lc)
	: _req(req), _lc(lc) {}

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

	bool rootHasSlash = (root[root.length - 1] == '/');
	bool pathHasSlash = (path[0] == '/');

	if (!rootHasSlash && !pathHasSlash)
		return root + "/" + path;
	
	if (rootHasSlash && pathHasSlash)
		return root + path.substr(1);
	return root + path;
}

// For GET

// /**
//  * @brief Gets the response Body from the requested file.
//  * 
//  * @note Relies on the internal target path storage (_fullpath).
//  * @return std::string The binary or text contents of the requested file.
//  * @throws HttpException 404 if the file cannot be opened.
//  */
// void Processor::extractBodyFromFile() {
// 	std::ifstream file(_fullpath.c_str(), std::ios::binary);
// 	if (!file)
// 		throw HttpException(404, "Not Found.");
// 	std::ostringstream ss;
// 	ss << file.rdbuf();
// 	_responseBody = s ss.str();
// }

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

/**
 * @brief Creates a file and fill it with the body parsed in the Http Request.
 * 
 * @throws HttpException 500 if ti fails creating th file.
 */
void Processor::createFile() {
	std::ofstream newFile(_fullpath);
	if (newFile.is_open()) {
		newFile << _requestBody;
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
	std::vector<std::string>::const_iterator it = indexes.begin()
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
const std::string Process::requestPath() {
	validateDir(_req._uri.path);
	return &_req._uri.path;
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

	DIR *folder = opendir(_fullpath.c_str());
	if (folder == NULL) {
		if (errno == EACCES)
			throw HttpException(403, "Forbidden");
		else if (errno == ENOENT)
			throw HttpException(404, "Not Found");
		else
			throw HttpException(500, "Internal Server Error");
	}
	std::stringstream html;
	std::string path = requestPath();

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
	bool isDir = validatePathDir();

	if (!isDir) {
			_extension = findFileExtension(_fullpath);
			_responseBody = readFile(_fullpath);
	}
	else {
		if (findIndexPage()) {
			_extension = findFileExtension(_fullpath);
			_responseBody = readFile(_fullpath);
		} else
			doAutoIndex();
		}
	}
	_code = 200;
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
	_code = 204;
	_codeMsg = "No Content";
	_responseBody.clear();
}

bool Processor::isValidMethod() {
	const std::vector<std::string> &methods = _lc.getAllowedMethods();
	std::vector<std::string>::iterator it = methods.begin();
	for (; it != methods.end(); ++it) {
		if (*it == _req.getMethod())
			return true;
	}
	return false;
}

// Routine
/**
 * @brief Derives the processing of the request to a handler depending on the Method.
 * 
 * @param method Method extracted in the Parse of the Http Request. 
 */
void Processor::processorRoutine() {
	_fullPath = concatPaths(_lc.getRoot(), _req.getPath());
	if (!isValidMethod())
		throw HttpException(405, "Method Not Allowed");
	// Redirect
	// CGI
	if (_req.getMethod() == "GET")
		handleGet();
	else if (_req.getMethod() ==  "POST")
		handlePost();
	else if (_req.getMethod() ==  "DELETE")
		handleDelete();
	else
       	throw HttpException(501, "Not Implemented");
}

void Processor::prepareResponse() {
	_responseBody = _response._responseBody;
	_response.assignHeaders(_extension, _request.getConnection());
}

// Getters.
/**
 * @brief Getter to full path variable saved in the object.
 * 
 * @return std::string The fullPath variable.
 */
std::string Processor::getFullPath() {
	return _fullPath;
}

/**
 * @brief Getter to the extension variable saved in the object.
 * 
 * @return std::string The Extension variable.
 */
std::string Processor::getExtension() {
	return _extension;
}

/**
 * @brief Getter to the body variable saved in the object.
 * 
 * @return std::string The Response Body variable.
 */
std::string Processor::getResponseBody() {
	return _responseBody;
}

/**
 * @brief Getter to the status code variable saved in the object.
 * 
 * @return std::string The Status Code variable.
 */
std::string Processor::getStatusCode() {
	return _code;
}
