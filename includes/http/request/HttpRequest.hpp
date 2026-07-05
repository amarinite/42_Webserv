#pragma once

#include <string>
#include <map>
#include <vector>

typedef struct {
	std::string methodHeader;
	t_uri		uri;
	std::string	HttpVer;
}	t_method;

class Request {
	private:
		HandleSocket	_socket;
		char			*_rawRequest;
		t_method		_method;
		std::map<std::string, std::string>  _headers;
		std::vector<char>	_body;

		//Functs
		//void recieveRawRequest();
		void parseRequest();
		// Will call this functs staticlly.
		// Since they cannot be called independently if've chosen this way.
		// - static void parseMethod();
		// - static void parseHeaders();
		// - static void parseBody();

	public:
		Request();
		Request(const HandleSocket &socket);
		Request(const Request &other);
		Request &operator=(const Request &other);
		~Request();

	//Getters
		HandleSocket	getSocket;
		t_method		getMethod;
		std::map<std::string, std::string>  getHeaders;
		std::vector<char>	getBody;
};

///////////////////////////////////////////////////////
// This will go to another file. 

enum class State {
	READING_HEADERS;
	READING_BODY;
	PROCESSING;
	WRITING_RESPONSE;
	FINISHED;
};

class Http {
	private:
		State		state;
		Request		&req;
		// Response	&res;

	public:
		//canonish;

}

// Acaba de decidir com fas el http general
// Acaba de dissenyar el flow de la maquina d'estats.
// Parseja requests.