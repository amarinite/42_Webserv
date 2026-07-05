#include "HttpRequest.hpp"

Request::Request() : _headRead(false), _bodyRead(false) {}

Request::Request(const Request &other) {
	this = other;
}

Request &Request::operator=(const Request &other) {
	if (this != &other) {
		this->_rawRequest = other._rawRequest;
		this->_methodHeader = other._methodHeader;
		this->_headers = other._headers;
		this->_body = other._body;
	}
	return *this;
}

Request::~Request() {}

// Functs

static void safeGetLine(std::stringstream &ss, std::string &leftover, char delimiter, bool &done) {
	std::string line;
	if (!std:;getline(ss, line, delimiter)) {
		if (ss.fail() || ss.bad())
			// 400 Bad Request.
		leftover += line;
		return;
	}
	leftover += line;
	done = true;
}

bool Request::parseMethod(std::stringstream &ss) {
	if (!this._methodParsed) {
		safeGetLine(ss, _method, ' ', this._methodParsed);
		if (!this._methodParsed)
			return 0;
		if (methodStr != "GET" && methodStr != "POST" && methodStr != "DELETE") // To be redone by server restrictions.
			// 400 Bad Request.
	}


	if (!this._uriParsed) {
		safeGetLine(ss, _uriStr, ' ', this._uriParsed);
		if (!this._uriParsed)
			return 0;
		this._uri = uriParser(_uriStr);
		this._uriStr.clear();
	}

	if (!this._httpVerParsed) {
		safeGetLine(ss, _httpVer, '\r', this._httpVerParsed);
		if (!this._httpVerParsed)
			return 0;
		if (httpVerStr != "HTTP/1.1")
			// 400 Bad Request.
		char peek = ss.peek();
		if (peek == '\n')
			ss.ignore();
		else
			// 400 Bad Request.
	}
	return true;
}

static void parseHeaders() {

}

static void parseBody();

bool Request::parseRequestHead(char *stream) {
	std::stringstream ss(stream);
	if (!this.parseMethod(ss))
		return false;
	this.parseHeaders(ss);
	return true;
}

bool Request::parseRequestBody(char *stream);





//Getters
t_method	Request::getMethod {
	return this->_method;
}

std::map<std::string, std::string>  Request::getHeaders {
	return this->_headers;
}

std::vector<char>	Request::getBody {
	return this->body;
}