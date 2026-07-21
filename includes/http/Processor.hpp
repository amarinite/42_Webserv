/**
 * @file Processor.hpp
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
 * @class Processor
 * @brief Fuse paths to get the complete route to the file,
 *		validates its exitance and extraxts exension and file contents.
 * 
 * Class that contains all necessary functs to process Http Requests.
 *
 */
class Processor {
	private:
		std::string		_fullPath;
		std::string		_extension;
		std::string		_responseBody;
		std::string		_code;
		std::string		_codeMsg;

		LocationConfig	_lc;
		Request			_req;
		// Response		_res;

	public:
		
		
		
		

		// Processor();
		- Fer el nomral i el de copia. 
		Processor(Request &req, LocationConfig &lc, Response &res);

		// Functs.
		std::string fullPath(std::string &root, std::string &path);
		void		validatePath();
		std::string findExtension();
		std::string extractBodyFromFile();

		// Getters
		std::string getFullPath();
};