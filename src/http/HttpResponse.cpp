#include "HttpResponse.hpp"

Response::Response() {
	_httpVer("HTTP/1.1");
}

Response::Response(const Response &other);
Response::Response &operator=(const Response &other);
Response::~Http();

void Response::assignHead(const HttpException& e) {
	_statusCode = e.getStatusCode();
	_message = e.getMessage();
}

void Response::assignHeaders() {
	_headers["Server: "] = "webserv";
	_headers["Date: "] = ;
	_headers["Content-Type: "] = ;
	_headers["Content-Length: "] = _responseBody.size();
	_Headers["Connection: "] = ;
}