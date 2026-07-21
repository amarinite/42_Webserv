#include "Processor.hpp"

/**
 * @brief Construct a new Processor:: Processor object
 * 
 * @param req Http Request Info
 * @param lc LocationConfig info
 * @param res Empty Response to store results.
 */
Processor::Processor(Request &req, LocationConfig &lc, Response &res)
	: _req(&req), _lc(&lc), _res(res) {}

/**
 * @brief Concatenates root directory with the requested directory
 			and saves it in the object.
 * 
 * @param root Base directory of the Processor.
 * @param path New directory extracted from the HTTP request.
 */
std::string Processor::fullPath(std::string &root, std::string &path) {
	if (!root.empty() && root[root.length() - 1] != '/' && (path.empty()
		|| path[0] != '/')) {
		_fullpath =  root + "/" + path;
	}
	_fullpath = root + path;
}

/**
 * @brief Detects if the path refers to a file or directory and
 *			if it has the according access rights.
 * 
 * @param fullpath the full path to analyze.
 * @return std::string indicating if it refers to a path or a file.
 */
std::string validatePathDir(const std::string &fullpath) {
	struct stat buff;
	if (stat(fullPath.c_str(), &buff) != 0)
		throw HttpException(404, "Not Found.");
	if (S_ISDIR(buff.st_mode)) {
		if (!(buff.st_mode & S_IRUSR))
			throw HttpException(403, "Forbidden.");
		return "DIR";
	}
	else if (S_ISREG(buff.st_mode)) {
		if (!(buff.st_mode & S_IRUSR))
			throw HttpException(403, "Forbidden.");
		return "FILE";
	}
	throw HttpException(403, "Forbidden.");
}

/**
 * @brief Checks if the requested file exists 
 		and if it has the adequate permissions.
 * @throws HttpException 404 if the path doesnt exist.
 * @throws HttpException 403 if the Processor has no access to it.
 */
void Processor::validateFile(const std::string &path) {
	struct stat buff;
	if (stat(path.c_str(), &buff) != 0)
		throw HttpException(404, "Not Found.");
	if (!(buffer.st_mode & S_IRUSR))
		throw HttpException(403, "Forbidden.");
}

/**
 * @brief Checks if the requested directory exists 
 		and if it has the adequate permissions.
 * @throws HttpException 404 if the path doesnt exist.
 * @throws HttpException 403 if the server has no access to it.
 */
void Processor::validateDir(const std::string &dir) {
	struct stat buff;
	if (stat(dir.c_str(), &buff) != 0)
		throw HttpException(404, "Not Found.");
	if (!S_ISDIR(buff.st_mode))
		throw HttpException(403, "Forbidden.");
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
 * @brief Gets the Extension of the requested file.
 * 
 * @note Relies on the internal target path storage (_fullpath).
 * @return std::string Extension of the file.
 * @throws HttpException 400 if the extension has a non contemplated length.
 */
std::string Processor::findExtension() {
	size_t dot = _fullpath.rfind('.');
		return _fullpath.substr(dot);
	if (dot == std::string::npos)
		return "";
	else if (dot == _fullpath.size() - 1)
		throw HttpException(400, "Bad Request: missing extension.");
}

/**
 * @brief Checks if the extension is valid cgi.
 * 
 * @param ext extension to verify.
 * @return true if it is a cgi extension.
 * @return false if it is not a cgi extension.
 */
bool Processor::CgiExtension(const std::string &ext) {
	std::map<std::string, std::string>::iterator it = _lc.getCgiExtension().find(ext);
	if (it != _lc.getCgiExtension().end())
		if (validateFile(it->second)
			return true;
	return false;
}

/**
 * @brief Checks if the extension is valid.
 * 
 * @param ext extension to verify.
 */
void Processor::fileExtension(const std::string &ext) {
	MimeTypes map;
	_extension = map.getType(ext);
}

// For DELETE
/**
 * @brief Removes a file from the system.
 *	
 * @throws HttpException 500 if std::remove() fails.
 */
void Processor::removeFile() {
	bool rm = std::remove(_fullpath);
	if (rm != 0)
		throw HttpException(500, "Internal Server Error: Couldnt remove file.");
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
bool findIndexPage() {
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
 * @brief Creates the autoindex page.
 * 
 * @throws HttpException 403 if user has no permits.
 * @throws HttpException 404 if directory doesnt exist.
 * @throws HttpException 500 if error of opendir.
 */
void Processor::doAutoIndex() {
	if (!_lc.hasAutoindex())
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

	html << "<html>\n<head><title>Index of " << _req.getPath() << "</title></head>\n";
	html << "<body style=\"font-family: sans-serif; padding: 20px;\">\n";
	html << "<h1>Index of " << _req.getPath() << "</h1>\n<hr>\n<ul>\n";

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
			findExtension();
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
	removeFile();
	_code = 204;
	_codeMsg = "No Content";
}

// Routine
/**
 * @brief Derives the processing of the request to a handler depending on the Method.
 * 
 * @param method Method extracted in the Parse of the Http Request. 
 */
void Processor::processorRoutine() Requeste I location {
	fullPath(_lc.getRoot(), _req.getPath());
	switch(method) {
		case GET:
			handleGet();
		case POST:
			handlePost();
		case DELETE:
			handleDelete();
	}
}

