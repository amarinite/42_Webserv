#pragma once

#include "Http.hpp"
#include "MimeTypes.hpp"
#include <ctime>
#include <map>

class Response {
	private:
		std::string							_statusCode;
		std::string 						_message;
		std::string							_responseBody;
		std::string							_connection;

		MimeTypes							_mimeMap;
		std::map<std::string, std::string>	_headers;

		std::vector<char>					_rawResponse;

	public:
		Response();
		~Response();

		//Functs
		std::string getTime();
		// void		assignHead(const HttpException& e);
		void		assignHeaders(std::string &extension);
		void		assignErrorBody(std::stirng &body);
		
	    Response    prepareErrorResponse(Response &res, std::map<int, std::string> &error_pages);
		void		prepareResponse();
		void		buildRawResponse();

		void setStatusCode(const std::string &code);
		void setMessage(const std::string &msg);
		void setResponseBody(const std::string &body);
		void setConnection(const std::string &conn);

		std::string getResponseBody();
};