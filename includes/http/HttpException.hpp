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
    std::string _methods;

public:
	HttpException(int code, const std::string& msg, const std::string &methods = "") : _statusCode(code), std::runtime_error(msg), _methods(methods) {}
	virtual ~HttpException() throw() {}

	virtual const char* what() const throw() {
        return _message.c_str();
    }

    int getStatusCode() const {
        return _statusCode;
    }

    Response    prepareErrorResponse(Response &res, std::map<int, std::string> &error_pages);
};