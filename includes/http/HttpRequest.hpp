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
	NO_BODY
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
		int			_client_max_body_size;
		bool		_chunkSize;
		size_t		_chunkTotal;

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
		// Request Line
		std::string	_stream;
		std::string _method;
		t_uri		_uri;

		// Headers
		std::map<std::string, std::string>  _headers;

		// Body
		std::string	_body;

		std::string _leftover;

		Request(int &clientMaxBodySize);
		Request(const Request &other);
		Request &operator=(const Request &other);
		~Request();

		// Public Functs
		bool parseRequestHead();
		bool parseRequestBody();
		
		// Getters
		std::string	getMethod();
		std::string	getBody();
		std::string getConnection() const;
		std::map<std::string, std::string>  getHeaders();

		// // Testing only
		// void feedStream(const std::string &data) {
		//     this->_stream += data;
		// }

		// void feedBody(const std::string &data) {
		//     this->_stream += data;
		// }
};