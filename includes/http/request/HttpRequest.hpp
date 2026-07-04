#pragma once

typedef struct {
	std::string methHeader;
	t_uri		uri;
	std::string	HttpVer;
}	t_method;

class Request {
	private:
		char	*_rawRequest;
		t_method	_method;
		std::map<std::string, std::string>  _header;
		std::vector<char>	_body;

		//Functs
		void recieveRawRequest();
		void parseRequest();
		// Will call this functs staticlly.
		// Since they cannot be called independently if've chosen this way.
		// - void parseMethod();
		// - void parseHeaders();
		// - void parseBody();

	public:
		Request();
		//Request(const char* rawRequest);
		Request(const Request &other);
		Request &operator=(const Request &other);
		~Request();

	//Getters
		t_method	getMethod;
		std::map<std::string, std::string>  getHead;
		std::vector<char>	getBody;
};