#pragma once

#include <string>
#include <map>
#include <vector>
#include <cctype>
#include <cstdlib>
#include "HttpException.hpp"
#include "UriParser.hpp"

enum BodyType {
	EMPTY,
	FULL,
	CHUNKED,
	OTHER
};

class Request {
	private:
		// Method
		std::string _uriStr;
		std::string	_httpVer;
		
		//Headers
		std::string _tmpKey;
		std::string _tmpVal;

		// Buffers
		std::string	_leftoverBody;

		// Head Parser
		bool _methodParsed;
		bool _uriParsed;
		bool _httpVerParsed;
		bool _parsedKey;
		bool _parsedValue;
		bool _incompleteEndLine;
		
		// Body Parse
		BodyType	_bodyType;
		size_t		_maxBodySize;
		bool		_chunkSize;

		//Functs
		// Head

		void checkInvalidHeaders();
		bool parseHeaders();
		void addHeader();
		bool findValue();
		bool findKey();
		bool parseMethod();
		bool safeEnd();

		// Body
		bool chunkedBody();
		bool fullBody();
		void setBodyType();
		
	public:
		std::string _leftover;
		std::string	_stream;
		std::string _method;
		t_uri		_uri;
		std::map<std::string, std::string>  _headers;
		
		//Body
		std::string	_body;

		Request();
		Request(const Request &other);
		Request &operator=(const Request &other);
		~Request();

		// Public Functs
		bool parseRequestHead();
		bool parseRequestBody();
		
		// Getters
		std::string	getMethod();
		std::map<std::string, std::string>  getHeaders();
		std::string	getBody();

		// // Testing only
		// void feedStream(const std::string &data) {
		//     this->_stream += data;
		// }

		// void feedBody(const std::string &data) {
		//     this->_stream += data;
		// }
};