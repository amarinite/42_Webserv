/**
 * @file Processor.hpp
 * @author jgirbau-
 * @brief all the necessary functs to process sttatic files.
 * @version 0.1
 * @date 2026-07-18
 */
#pragma once
#include "Http.hpp"
#include "FileUtils.hpp"
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

		// Functs.
		void convertFileExtension(const std::string &ext);
		void createFile();
		void doAutoIndex();
		void handleGet();
		void handlePost();
		void handleDelete();
		bool findIndexPage();
		bool isValidMethod();

	public: 
		// Constructor.
		Processor(Request &req, LocationConfig &lc);

		// Functs.
		void processorRoutine();

		// Getters.
		std::string getFullPath();
};