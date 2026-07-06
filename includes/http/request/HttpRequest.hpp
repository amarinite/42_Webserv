#pragma once

#include <string>
#include <map>
#include <vector>

class Request {
	private:
	
		std::string _stream;
		char *_streamLeftover;

		std::string _method;
		t_uri		_uri;
		std::string	_httpVer;
		
		std::map<std::string, std::string>  _headers;
		std::vector<char>	_body;
		
		bool _methodParsed;
		bool _uriParsed;
		bool _httpVerParsed;
		
		bool _headRead;
		bool _bodyRead;

		std::string _uriStr;
		std::string _leftover;

		bool _parsedKey;
		bool _parsedValue;

		size_t _streamLeft;
		

		//Functs
		//void recieveRawRequest();
		bool parseRequestHead(char *stream);
		bool parseRequestBody(char *stream);
		// Will call this functs staticlly.
		// Since they cannot be called independently if've chosen this way.
		bool parseMethod(std::stringstream &ss);
		static bool findKey(std::string &key);
		static bool findValue(std::string &value);
		static void addHeader(bool key, bool value)
		// - static void parseHeaders();
		// - static void parseBody();

	public:
		Request();
		Request(const Request &other);
		Request &operator=(const Request &other);
		~Request();

	//Getters
		
		t_method		getMethod;
		std::map<std::string, std::string>  getHeaders;
		std::vector<char>	getBody;
};

///////////////////////////////////////////////////////

// Acaba de decidir com fas el http general
// Acaba de dissenyar el flow de la maquina d'estats.
// Parseja requests.