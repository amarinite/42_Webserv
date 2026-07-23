#pragma once

#include "Http.hpp"
#include "FileUtils.hpp"
#include <exception>
#include <string>
#include <cstdio>

extern bool exceptConnection;

class HttpException : public std::runtime_error {
private:
    int         _statusCode;
    std::string _message;

public:
	HttpException(int code, const std::string& msg) : _statusCode(code), std::runtime_error(msg) {}
	virtual ~HttpException() throw() {}

	virtual ~HttpException() throw() {}

    int getStatusCode() const {
        return _statusCode;
    }

    Response    prepareErrorResponse();
};