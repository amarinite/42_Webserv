#pragma once

#include "Http.hpp"
#include "MimeTypes.hpp"
#include <ctime>
#include <map>

class Response {
	private:
		void assignHead(const HttpException& e);

	public:
		std::string			_statusCode;
		std::string 		_message;
		const std::string 	_responseBody;
		const std::string	_connection;
		
		char*				_response;
		size_t				_responseSize;

		MimeTypes			_mimeMap;
		

		std::map<std::string, std::string> _headers;

		Response(std::string &body);
		// Response(const Response &other);
		// Response &operator=(const Response &other);

		//Functs
		std::string getTime();
		void assignHead(const HttpException& e);
		void assignHeaders(std::string &extension);
		void assignBody(std::stirng &body);
		void buildRawResponse();
};