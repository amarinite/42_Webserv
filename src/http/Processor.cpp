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
static std::string concatPaths(std::string &root, std::string &path) {
	if (!root.empty() && root[root.length() - 1] != '/' && (path.empty()
		|| path[0] != '/')) {
		return  root + "/" + path;
	}
	return root + path;
}

/**
 * @brief Returns the full path variable saved in the object.
 * 
 * @return std::string The fullPath variable.
 */
std::string Processor::getFullPath() {
	return _fullPath;
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
void Processor::handleMultipart(std::string &content) {
	size_t pos = content.find("boundary")
	if (pos == content.end())
		throw HttpException(400, "Bad Request: Bad Header");
	std::string boundary = "--" + content.substr(10);
}

void Processor::handleContentType() {
	std::map<std::string, std::string>::iterator it = _req._headers.find("content-type");
	if (it == _req._headers.end())
		throw HttpException(400, "Bad Request: Missing Content-Type header");
	if (it->second.find("multipart/form-data"))
		handleMultipart(it->second);
	else if (it->second.find("application/x-www-form-urlencoded"))
		handleXForm(it->second);
	else if (it->second.find("text/plain") || it->second.find("application/octet-stream"))
		handlePlainTxt(it->second);
}

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
 * @return true If finds a correct file.
 * @return false  if it doesnt find a correct file.
 */
bool Processor::findIndexPage() {
	std::vector<std::string>::iterator it = _lc.getIndex().begin();
	for (; it != _lc.getIndex().end(); ++it) {
		std::string potentialIdx = fullPath(_lc.getRoot(), it);
		try {
			if (validateFile(potentialIdx)) {
				_fullPath = potentialIdx;
				return true;
			}
		} catch (...) {}
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
		html << "<li><a>" << content->d_name << "</a></li>\n";
	html << "</ul>\n<hr>\n</body>\n</html>";

	_responseBody = html.str();
	closedir(folder);
}

/**
 * @brief Unifies de functions of GET method and sets the Status code and message.
 */
void Processor::handleGet() {
	std::string pathType = validatePathDir();
	switch (pathType) {
		case FILE:
			_extension = findFileExtension(_fullpath);
			extractBodyFromFile();
		case PATH:
			if (!findIndexPage())
				doAutoIndex();
	}
	_code = 200;
	_codeMsg = "Ok";
}

/**
 * @brief Unifies de functions of POST method and sets the Status code and message.
 */
void Processor::handlePost() {
	validateDirectory();
	createFile();
	_code = 201;
	_codeMsg = "Created";
}

/**
 * @brief Unifies de functions of DELETE method and sets the Status code and message.
 */
void Processor::handleDelete() {
	validateFile();
	removeFile(_fullpath);
	_code = 204;
	_codeMsg = "No Content";
}

bool Processor::isValidMethod() {
	std::vector<std::string>::iterator it = _lc.getAllowedMethods().begin();
	for (; it != _lc.getAllowedMethods().end(); ++it) {
		if (*it == _request._method)
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
	_fullpath = concatPaths(_lc.getRoot(), _req.getPath());
	if (!isValidMethod())
		throw HttpException(405, "Method Not Allowed");
	// Redirect
	// CGI
	switch(method) {
		case GET:
			handleGet();
		case POST:
			handlePost();
		case DELETE:
			handleDelete();
	}
}
