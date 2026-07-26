#pragma once

#include <stdexcept>
#include <string>

extern bool exceptConnection;

class HttpException : public std::runtime_error {
private:
	int			_statusCode;
	std::string _message;
	std::string _methods;

public:
	HttpException(int code, const std::string& msg, const std::string &methods = "") : 
		std::runtime_error(msg),	
		_statusCode(code),
		_message(msg),
		_methods(methods)
	{}

	virtual ~HttpException() throw() {}

	virtual const char* what() const throw() {
		return _message.c_str();
	}

	int getStatusCode() const {
		return _statusCode;
	}

	const std::string &getMethods() const {
		return _methods;
	}
};