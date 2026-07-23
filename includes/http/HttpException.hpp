#pragma once

#include <exception>
#include <string>

class HttpException : public std::runtime_error {
private:
	int			_statusCode;

public:
	HttpException(int code, const std::string& msg) : _statusCode(code), std::runtime_error(msg) {}
	virtual ~HttpException() throw() {}

	virtual ~HttpException() throw() {}

	int getStatusCode() const {
		return _statusCode;
	}
};