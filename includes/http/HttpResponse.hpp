#pragma once

// #include "Http.hpp"
#include "HttpException.hpp"
#include "MimeTypes.hpp"
#include "ErrorMsg.hpp"
#include "FileUtils.hpp"
#include <ctime>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include <unistd.h>

class Response {
	private:
		std::string							_statusCode;
		std::string 						_message;
		std::string							_responseBody;
		std::string							_connection;

		MimeTypes							_mimeMap;
		ErrorMsg							_errMsg;
		std::map<std::string, std::string>	_headers;

		std::vector<char>					_rawResponse;

	public:
		Response();
		~Response();

		// Helpers
		std::string getTime();
		void		assignHeaders(const std::string &extension, const std::string &connection);
		void		assignErrorBody(const size_t &statusCode, const std::map<int, std::string> &error_pages);
		void		errorBody(const std::string &statusCode, const std::string &errorDir);
		void		buildRawResponse();
		void		prepareErrorResponse(const std::map<int, std::string> &error_pages, HttpException &ex);		

		// Setters
		void		setStatusCode(const std::string &code);
		void		setMessage(const std::string &msg);
		void 		setResponseBody(const std::string &body);
		void		setConnection(const std::string &conn);
		void		setLocationHeader(const std::string &location);
		void		setAllowedMethodsHeader(const std::string &allowed);

		// Getters
		const std::string		&getResponseBody() const;
		const std::vector<char>	&getRawResponse() const;
};