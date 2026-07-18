/**
 * @file StaticFiles.hpp
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
 * @class Static (in short for Static Files.)
 * @brief Fuse paths to get the complete route to the file,
 *		validates its exitance and extraxts exension and file contents.
 * 
 * Class that contains all necessary functs to process static files.
 *
 */
class Static {
	public:
		std::string _fullPath;

		Static();

		// Functs.
		std::string fullPath(std::string &root, std::string &path);
		void		validatePath();
		std::string findExtension();
		std::string extractBodyFromFile();

		// Getters
		std::string getFullPath();
};