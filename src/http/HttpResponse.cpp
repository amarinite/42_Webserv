#include "HttpResponse.hpp"

Response::Response() {
	_mimeMap;
}

// void Response::assignHead(const HttpException& e) {
// 	_statusCode = e.getStatusCode();
// 	_message = e.getMessage();
// }

template <typename T>
std::string toStr(const T &num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::string Response::getTime() {
	char buff[100];
	std::time_t now = std::time(NULL);
	std::tm	*gmt = std::gmtime(&now);
	if (gmt == NULL)
		throw HttpException(500, "Internal Server Error.");
	if (std::strftime(buff, sizeof(buff), "%a, %d %b %Y %H:%M:%S GMT", gmt))
		return std::string(buff);
	throw HttpException(500, "Internal Server Error.");
}

void Response::assignHeaders(std::string &extension, std::string &connection) {
	_headers["Server: "] = "Group de Afectadous by Taha";
	_headers["Date: "] = getTime();
	_headers["Connection: "] = connection;
	if (!_responseBody.empty()) {
		_headers["Content-Type: "] = _mimeMap.getType(extension);
		_headers["Content-Length: "] = toString(_responseBody.size());
	}
}

void Response::errorBody(const std::string &statusCode, std::string &errorDir) {
	std::string errPage = errorDir;
	if (!errPage.empty() && errPage[errPage.size() - 1] != '/') {
		errPage += "/";
	}
	errPage += statusCode + ".html";
	try {
		_responseBody = readFile(errPage);
	} catch (...) {
		_statusCode = 500;
		_message = "Internal Server Error";
		_responseBody = "<h1>500 Internal Server Error</h1>";
	}
}

void Response::assignErrorBody(size_t &statusCode, const std::map<int, std::string> &error_pages) {
	if (statusCode > 399) {
		_responseBody = errorBody(toStr(statusCode), error_pages[statusCode]);
	}
}

void Response::buildRawResponse() {
	std::ostringstream oss;
	oss << "HTTP/1.1 " << _statusCode << " " << _message << "\r\n";
	std::map<std::string, std::string>::iterator it = _headers.begin();
	for (; it != _headers.end(); ++it) {
		oss << it->first << ": " << it->second << "\r\n";
	}
	oss << "\r\n";
	if (!_responseBody.empty())
		oss << _responseBody;	
	std::string fullResponse = oss.str();
	_rawResponse.assign(fullResponse.begin(), fullResponse.end());
}

// Case 301 - Redirect
void Response::setLocationHeader(const std::string &location) {
	if (!location.empty())
		_headers["Location: "] = location;
}

// // Case 405 - Not allowed method.
void Response::setAllowedMethodsHeader(const std::string &allowed) {
	_headers["AllowedMethods: "] = allowed;
}


// void	Response::prepareErrorResponse(HttpException &exc, std::map<int, std::string> &error_pages) {
// 	Response res();
// 	std::string strStatusCode = intToString(exc.getStatusCode());

// 	res.setStatusCode(strStatusCode);
// 	res.setMessage(responseStatusMessage(strStatusCode));
// 	std::string body = errorPageBody(exc.getStatusCode(), error_pages);
// 	res.setResponseBody(body);
// 	res.assignHeaders("text/html", setConnection(exc.getStatusCode()));
// }

// Setters.
void Response::setStatusCode(const std::string &code) {
	_statusCode = code;
}

void Response::setMessage(const std::string &msg) {
	_message = msg;
}

void Response::setResponseBody(const std::string &body) {
	_responseBody = body;
}

void Response::setConnection(const std::string &conn) {
	_connection = conn;
}

std::string Response::getResponseBody() {
	return _responseBody;
}
