#include "Server.hpp"

/**
 * @brief Construct a new Server:: Server object
 */
Server::Server() {}

/**
 * @brief Concatenates root directory with the requested directory
 			and saves it in the object.
 * 
 * @param root Base directory of the server.
 * @param path New directory extracted from the HTTP request.
 */
std::string Server::fullPath(std::string &root, std::string &path) {
	if (!root.empty() && root[root.length() - 1] != '/' && (path.empty()
		|| path[0] != '/')) {
		_fullpath =  root + "/" + path;
	}
	_fullpath = root + path;
}

/**
 * @brief Checks if the requested file exists 
 		and if it has the adequate permissions.
 * @throws HttpException 404 if the path doesnt exist.
 * @throws HttpException 403 if the server has no access to it.
 */
void Server::validatePath() {
	struct stat buff;
	if (stat(_fullPath.c_str(), &buff) != 0)
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
void Server::validateDir() {
    struct stat buff;
	if (stat(_fullPath.c_str(), &buff) != 0)
		throw HttpException(404, "Not Found.");
    if (!S_ISDIR(buff.st_mode))
        throw HttpException(403, "Forbidden.");
}

/**
 * @brief Returns the full path variable saved in the object.
 * 
 * @return std::string The fullPath variable.
 */
std::string Server::getFullPath() {
	return _fullPath;
}

// For GET

/**
 * @brief Gets the response Body from the requested file.
 * 
 * @note Relies on the internal target path storage (_fullpath).
 * @return std::string The binary or text contents of the requested file.
 * @throws HttpException 404 if the file cannot be opened.
 */
std::string Server::extractBodyFromFile() {
	std::ifstream file(_fullpath.c_str(), std::ios::binary);
	if (!file)
		throw HttpException(404, "Not Found.");
	std::ostringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

/**
 * @brief Gets the Extension of the requested file.
 * 
 * @note Relies on the internal target path storage (_fullpath).
 * @return std::string Extension of the file.
 * @throws HttpException 400 if the extension has a non contemplated length.
 */
std::string Server::findExtension() {
	size_t dot = _fullpath.rfind('.');
	// if (dot == std::string::npos || dot == _fullpath.size() - 1)
	// 	throw HttpException(400, "Bad Request: missing extension.");
	if ((_fullpath.size() - dot) > 5)
		throw HttpException(400, "Bad Request: bad extension.");
	MimeTypes map;
	return map.getType(_fullpath.substr(dot));
}

// For DELETE
/**
 * @brief Removes a file from the system.
 *	
 * @throws HttpException 500 if std::remove() fails.
 */
void Server::removeFile() {
	bool rm = std::remove(_fullpath);
	if (rm != 0)
		throw HttpException(500, "Internal Server Error: Couldnt remove file.");
}

std::string Server::messageDelete() {
	return "No Content";
}

size_t Server::codeDelete() {
	return 204;
}

// For POST
/**
 * @brief Creates a file and fill it with the body parsed in the Http Request.
 * 
 * @throws HttpException 500 if ti fails creating th file.
 */
void Server::createFile() {
	std::ofstream newFile(_fullpath);
	if (newFile.is_open()) {
		newFile << _requestBody;
		newFile.close();
	} else
		throw HttpException(500, "Internal Server Error: error creating file.");
}

std::string Server::messageDelete() {
	return "Created";
}

size_t Server::codeDelete() {
	return 201;
}

// Routine
void Server::routine() {
    
}