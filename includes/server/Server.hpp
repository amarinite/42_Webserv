/**
 * @file Server.hpp
 * @author jgirbau-
 * @brief all the necessary functs to process sttatic files.
 * @version 0.1
 * @date 2026-07-18
 */
#pragma once
#include "Http.hpp"
#include <sys/stat.h>
#include <fstream>
#include <sstream>

/**
 * @class Server (in short for Server Files.)
 * @brief Fuse paths to get the complete route to the file,
 *		validates its exitance and extraxts exension and file contents.
 * 
 * Class that contains all necessary functs to process Server files.
 *
 */
class Server {
	public:
		std::string _fullPath;
        std::string _extension;
        std::string _responseBody;
        std::string _code;
		std::string _codeMsg;

		Server();

		// Functs.
		std::string fullPath(std::string &root, std::string &path);
		void		validatePath();
		std::string findExtension();
		std::string extractBodyFromFile();

		// Getters
		std::string getFullPath();
};