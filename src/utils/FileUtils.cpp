#include "FileUtils.hpp"

std::string readFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		throw std::runtime_error("could not open config file: " + path);

	std::stringstream buffer;
	buffer << file.rdbuf();

	if (file.bad())
		throw std::runtime_error("error reading config file: " + path);

	return buffer.str();
}

/**
 * @brief Detects if the path refers to a file or directory and
 *			if it has the according access rights.
 * 
 * @param path the full path to analyze.
 * @return true if it is a directory, false if it is a regular file.
 */
bool validatePathDir(const std::string &path) {
	struct stat buff;
	if (stat(path.c_str(), &buff) != 0)
		throw HttpException(404, "Not Found.");
	if (S_ISDIR(buff.st_mode)) {
		if (access(path.c_str(), R_OK | X_OK) != 0)
			throw HttpException(403, "Forbidden.");
		return true; // Is dir.
	}
	else if (S_ISREG(buff.st_mode)) {
		if (access(path.c_str(), R_OK) != 0)
			throw HttpException(403, "Forbidden.");
		return false; // Is file.
	}
	throw HttpException(403, "Forbidden.");
}

/**
 * @brief Checks if the requested file exists 
 *		and if it has the adequate permissions.
 *
 * @param path the full path to analyze.
 * @throws HttpException 404 if the path doesnt exist.
 * @throws HttpException 403 if the Processor has no access to it.
 */
bool validateFile(const std::string &path) {
	struct stat buff;
	if (stat(path.c_str(), &buff) != 0)
		throw HttpException(404, "Not Found.");
	if (!S_ISREG(buff.st_mode))
        throw HttpException(403, "Forbidden: Target is not a regular file.");
	if (access(path.c_str(), R_OK) != 0)
		throw HttpException(403, "Forbidden.");
	return true;
}

/**
 * @brief Checks if the requested directory exists 
 * 		and if it has the adequate permissions.
 *
 * @param path the full path to analyze.
 * @throws HttpException 404 if the path doesnt exist.
 * @throws HttpException 403 if the server has no access to it.
 */
bool validateDir(const std::string &dir) {
	struct stat buff;
	if (stat(dir.c_str(), &buff) != 0)
		throw HttpException(404, "Not Found.");
	if (!S_ISDIR(buff.st_mode))
		throw HttpException(403, "Forbidden.");
	return true;
}

/**
 * @brief Gets the Extension of the requested file.
 * 
* @param path the full path to analyze.
 * @note Relies on the internal target path storage (_fullpath).
 * @return std::string Extension of the file.
 * @throws HttpException 400 if the extension has a non contemplated length.
 */
std::string findFileExtension(std::string &path) {
	size_t lastSlash = path.rfind('/');
	size_t dot = path.rfind('.');
	if (dot == std::string::npos || (lastSlash != std::string::npos && dot < lastSlash))
		return "";
	else if (dot == path.size() - 1)
		throw HttpException(400, "Bad Request: missing extension.");
	return path.substr(dot);
}

/**
 * @brief Removes a file from the system.
 *	
 * @param path the full path to analyze.
 * @throws HttpException 500 if std::remove() fails.
 */
void removeFile(std::string &path) {
	if (access(_fullPath.c_str(), W_OK) != 0) {
        throw HttpException(403, "Forbidden: No write permissions to delete file.");
    }
	if (std::remove(path.c_str()) != 0)
		throw HttpException(500, "Internal Server Error: Couldnt remove file.");
}