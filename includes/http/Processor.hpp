/**
 * @file Processor.hpp
 * @author jgirbau-
 * @brief all the necessary functs to process sttatic files.
 * @version 0.1
 * @date 2026-07-18
 */
#pragma once

#include <sys/stat.h>
#include <fstream>
#include <sstream>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "LocationConfig.hpp"

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

		
		Request					&_req;
		Response				&_res;
		LocationConfig	_lc;	

		// Functs.
		void handleGet();
		void handlePost();
		void handleDelete();
		void convertFileExtension(const std::string &ext);
		void createFile();
		void doAutoIndex();
		bool findIndexPage();
		bool isValidMethod();

		const std::string requestPath() const;

	public: 
		// Constructor.
		Processor();
		Processor(Request &req, Response &res, const LocationConfig &lc);
		//Processor &operator=(const Processor &p);

		// Functs.
		void		processorRoutine();
		void		prepareResponse();

		// Getters.
		const std::string &getFullPath() const;
		const std::string &getExtension() const;
		const std::string &getResponseBody() const;
		const std::string &getStatusCode() const;
};