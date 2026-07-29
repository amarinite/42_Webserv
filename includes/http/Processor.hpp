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
#include <unistd.h>

#include "cgi/CgiHandler.hpp"
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
		std::string				_fullPath;
		std::string				_extension;
		std::string				_responseBody;
		std::string				_code;
		std::string				_codeMsg;
		std::string				_redirectPath;

        bool					_cgiRequested;
		std::string				_cgiScriptPath;
		std::string				_cgiExecPath;

		const LocationConfig	&_lc;
		Request					&_req;
		Response				&_res;

		// Functs.
		void convertFileExtension(const std::string &ext);
		void doAutoIndex();
		void handleGet();
		void handlePost();
		void handleDelete();
        void prepareCgi();
		bool findIndexPage();
		bool isValidMethod();
		bool isRedirect() const;
		void handleRedirect();

		const std::string requestPath() const;

	public:
		// Constructor.
		Processor(Request &req, Response &res, const LocationConfig &lc);

		// Functs.
		void processorRoutine();
        void prepareResponse();
		void consumeCgiOutput(const std::string &rawCgiOutput);

        bool wantsCgi() const;
		const std::string &getCgiScriptPath() const;
		const std::string &getCgiExecPath() const;

		// Getters.
		const std::string &getFullPath() const;
		const std::string &getExtension() const;
		const std::string &getResponseBody() const;
		const std::string &getStatusCode() const;
};
