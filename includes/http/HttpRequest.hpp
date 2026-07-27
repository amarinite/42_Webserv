#pragma once

#include <string>
#include <map>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <cstddef>

#include "UriParser.hpp"

enum BodyType {
	EMPTY,
	FULL,
	CHUNKED,
	NO_BODY
};

class Request {
	private:
		// Request Line
		std::string	_stream;
		std::string _method;
		std::string _uriStr;
		std::string	_httpVer;
		t_uri		_uri;

		//Headers
		std::map<std::string, std::string>  _headers;
		std::string _tmpKey;
		std::string _tmpVal;

		// Buffers
		std::string	_body;
		std::string _leftover;
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
		size_t	_client_max_body_size;
		bool		_chunkSize;
		size_t		_chunkTotal;

		// Extra
		std::vector<std::string>*	_allowedMethods;

		//Functs
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
		Request(size_t clientMaxBodySize);
		// Request(const Request &other);
		// Request &operator=(const Request &other);
		~Request();

		// Public Functs
		bool parseRequestHead();
		bool parseRequestBody();
		
		// Getters
		const std::string	&getMethod() const;
		const std::string	&getBody() const;
		const std::string	&getPath() const;
		std::string			getLeftover();
		std::string			getConnection() const;
		const t_uri			&getUri() const;
		const std::map<std::string, std::string>&  getHeaders() const;

		void setStream(const std::string &stream);

		void clearLeftover();
};