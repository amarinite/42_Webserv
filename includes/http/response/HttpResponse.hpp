#pragma once

#include "Http.hpp"
#include "MimeTypes.hpp"
#include <ctime>
#include <map>

class Response {
	private:
		void assignHead(const HttpException& e);

	public:
		const std::string	_httpVer;
		size_t				_statusCode;
		std::string 		_message;
		const std::string 	_responseBody;
		const std::string	_connection;
		
		std::map<std::string, std::string> _headers;

		char		*_rawResponse;
		MimeTypes	_mimeMap;


		Response();
		Response(std::string &body);
		// Response(const Response &other);
		// Response &operator=(const Response &other);
		~Http();

		//Functs
		std::string getTime();
		void assignHead(const HttpException& e);
		void assignHeaders(std::string &extension);
		void assignBody(std::stirng &body);
		void buildRawResponse();
};