#include "HttpResponse.hpp"

Response::Response() {}

Response::~Response() {}

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

void Response::assignHeaders(const std::string &extension, const std::string &connection) {
	_headers["Server: "] = "Group de Afectadous by Taha";
	_headers["Date: "] = getTime();
	_headers["Connection: "] = connection;
	if (!_responseBody.empty()) {
		_headers["Content-Type: "] = _mimeMap.getType(extension);
		_headers["Content-Length: "] = toStr(_responseBody.size());
	}
}

// Case 301 - Redirect
void Response::setLocationHeader(const std::string &location) {
	if (!location.empty())
		_headers["Location: "] = location;
}

// Case 405 - Not allowed method.
void Response::setAllowedMethodsHeader(const std::string &allowed) {
	_headers["Allow: "] = allowed;
}

void Response::errorBody(const std::string &statusCode, const std::string &errorDir) {
	std::string errPage = errorDir;
	if (!errPage.empty() && errPage[errPage.size() - 1] != '/') {
		errPage += "/";
	}
	errPage += statusCode + ".html";
	try {
		_responseBody = readFile(errPage);
	} catch (...) {
		_statusCode = toStr(500);
		_message = "Internal Server Error";
		_responseBody = "<h1>500 Internal Server Error</h1>";
	}
}

void Response::assignErrorBody(const size_t &statusCode, const std::map<int, std::string> &error_pages) {
	if (statusCode > 399) {
		std::map<int, std::string>::const_iterator it = error_pages.find(static_cast<int>(statusCode));

		if (it != error_pages.end())
			errorBody(toStr(statusCode), it->second);
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



// // Error Response
// static std::string errorPageBody(const int errorCode, std::map<int, std::string> &error_pages) {
// 	std::map<int, std::string>::iterator it = error_pages.begin();
// 	for (; it != error_pages.end(); ++it) {
// 		if (it->first == errorCode)
// 			return readFile(it->second);
// 	}
// 	return "";
// }

static std::string setErrorConnection(const int &code) {
	if (code == 400 || code == 413 || code > 499 || !exceptConnection)
		return "close";
	else
		return "keep-alive";
}

// conf.getErrorPages()
void	Response::prepareErrorResponse(const std::map<int, std::string> &error_pages, const HttpException &ex) {
	std::string strStatusCode = toStr(ex.getStatusCode());

	setStatusCode(strStatusCode);
	setMessage(_errMsg.getErrorMsg(ex.getStatusCode()));
	assignErrorBody(ex.getStatusCode(), error_pages);
	assignHeaders(".html", setErrorConnection(ex.getStatusCode()));
	if (ex.getStatusCode() == 405)
		setAllowedMethodsHeader(ex.getMethods());
	buildRawResponse();
}

void Response::addRawHeader(const std::string &key, const std::string &value) {
	_headers[key] = value;
}

void Response::assignConnectionAndLengthHeaders(const std::string &connection) {
	_headers["Server"] = "Group de Afectadous by Taha";
	_headers["Date"] = getTime();
	_headers["Connection"] = connection;
	if (!_responseBody.empty())
		_headers["Content-Length"] = toStr(_responseBody.size());
}

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

const std::string &Response::getResponseBody() const {
	return _responseBody;
}

const std::vector<char> &Response::getRawResponse() const {
	return _rawResponse;
}

const std::string &Response::getStatusCode() const {
	return _statusCode;
}