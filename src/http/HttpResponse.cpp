#include "HttpResponse.hpp"

Response::Response() {
	_mimeMap();
}

// sResponse::~Http();

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

void Response::assignHeaders(std::string &extension, std::string &location) {
	_headers["Server: "] = "Group de Afectadous por Taha";
	_headers["Date: "] = getTime();
	_headers["Connection: "] = _request.getConnection();
	if (!_responseBody.empty()) {
		_headers["Content-Type: "] = _mimeMap.getType(extension);
		_headers["Content-Length: "] = _responseBody.size();
	}
}

std::string errorBody(size_t &statusCode, std::string &errorDir) {
	std::string errPage = errorDir + statusCode + ".html";
	try {
		_responseBody = readFile(errPage);
	} catch (...) {
		_statusCode = 500;
		_message = "Internal Server Error";
		_responseBody = "<h1>500 Internal Server Error</h1>";
	}
}

///////////////Hem d'entrar a response amb el body definitiu ja guardat a la classe.
void Response::assignBody() {
	if (_statusCode > 399)
		_responseBody = errorBody(_statusCode);
}

void Response::buildRawResponse() {
	std::ostringstream oss;
	oss << "HTTP/1.1 " << _statusCode << " " << _message << "\r\n";
	std::map<std::string, std::string>::iterator it = _headers.begin();
	for (; it != _headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n"
	}
	oss << "\r\n";
	if (_responseBody)
		oss << _responseBody;	
	std::string fullResponse = oss.str();
	std::vector<char> buff(fullResponse.begin(), fullResponse.end());
	_response = &buff[0];
	_responseSize = buff.size();
}

// Case 301 - Redirect
void Response::setLocationHeader(std::string &location) {
	_headers["Location: "] = location;
}

// Case 405 - Not allowed method.
void Response::setAllowedMethodsHeader(std::vector<std::string> &allowed) {
	std::ostringstream oss;
	for (size_t i = 0; i < allowed.size(); ++i) {
		if (i != 0)
			oss << ", "
		oss << allowed[i];
	}
	_headers["Allow: "] = oss.str();
}

void prepareResponse() {
	
}

// 400, 404, 405, 403, 500, 502 503, 504  
// 301 200 204

// Rutina Processor
// Rutina Response
// Assegurarse que el body esta complet abans de entra a construir la resposta. 
// Doxygen?