#include "HttpResponse.hpp"

Response::Response() {
	_httpVer("HTTP/1.1");
	_mimeMap();
}

Response::~Http();

void Response::assignHead(const HttpException& e) {
	_statusCode = e.getStatusCode();
	_message = e.getMessage();
}

std::string Response::getTime() {
	char buff[100];
	std:::time_t whatTimeIsIt = std::time(NULL);
	std::tm	*summerTime;
	if (std::strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S GMT", summerTime))
		return std::string(buff);
	return "Not available";
}

void Response::assignHeaders(std::string &extension) {
	// 204, 304
	_headers["Server: "] = "webserv42";
	_headers["Date: "] = getTime();
	_headers["Connection: "] = _request.getConnection();
	_headers["Content-Type: "] = _mimeMap.getType(extension);
	_headers["Content-Length: "] = _responseBody.size();
	
}

void Response::assignBody(std::string &body) {
	_responseBody = body;
}

void Response::assignBody() {
	
}

void Response::buildRawResponse() {
	_rawResponse = 

}

// 400, 404, 405, 403, 500, 502 503, 504  
// 301 200 204