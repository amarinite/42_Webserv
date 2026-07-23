#pragma once

#include "Http.hpp"
#include "MimeTypes.hpp"
#include <ctime>
#include <map>

class Response {
	private:
		std::string			_statusCode;
		std::string 		_message;
		const std::string 	_responseBody;
		const std::string	_connection;

		MimeTypes			_mimeMap;

		void assignHead(const HttpException& e);

	public:
		char*				_response;
		size_t				_responseSize;

		std::map<std::string, std::string> _headers;

		Response();
		// Response(const Response &other);
		// Response &operator=(const Response &other);

		//Functs
		std::string getTime();
		void assignHead(const HttpException& e);
		void assignHeaders(std::string &extension);
		void assignBody(std::stirng &body);
		void buildRawResponse();

		void setStatusCode();
		void setMessage();
		void setResponseBody();
		void setConnection();
};