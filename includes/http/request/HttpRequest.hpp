#pragma once

#include <string>
#include <map>
#include <vector>

enum BodyType {
	EMPTY,
    FULL,
    CHUNKED,
	OTHER
};

class Request {
	private:
		// Method
		std::string _method;
		t_uri		_uri;
		std::string	_httpVer;
		
		//Headers
		std::map<std::string, std::string>  _headers;

		//Body
		std::vector<char>	_body;

		// Comprovacions i validacions
		std::string		_stream;
		std::string		_streamBody;

		// Head Parser
		bool _methodParsed;
		bool _uriParsed;
		bool _httpVerParsed;

		std::string _uriStr;
		std::string _leftover;

		bool _parsedKey;
		bool _parsedValue;

		size_t	_streamLeft;
		bool	_incompleteEndLine;
		
		std::string _tmpKey;
		std::string _tmpVal;

		// Body Parse
		int				_bodyType;
		size_t			_maxBodySize;
		std::string		_leftoverBody;
		bool			_chunkSize;

		//Functs
		//void recieveRawRequest();
		bool parseRequestHead();
		bool parseRequestBody();
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
		
		t_method	getMethod;
		std::map<std::string, std::string>  getHeaders;
		std::vector<char>	getBody;
};

///////////////////////////////////////////////////////

// Acaba de decidir com fas el http general
// Acaba de dissenyar el flow de la maquina d'estats.
// Parseja requests.