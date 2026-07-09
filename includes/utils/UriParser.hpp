#pragma once

#include <string>
#include <iostream>
#include <cctype>
#include <cstring>
#include <sstream>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

typedef struct {
	bool		has_scheme;
	std::string scheme;

	bool		has_auth;
	std::string authority;
	int			auth_port;

	bool		has_path;
	std::string path;

	bool		has_query;
	std::string query;

	bool		has_frag;
	std::string fragment;

	t_uri()
		: has_scheme(false), scheme(),
			has_auth(false), authority(), auth_port(0),
			has_path(false), path(),
			has_query(false), query(),
			has_frag(false), fragment()
	{}
	
}	t_uri;

void	parseUri(t_uri &uri, std::string req);

#include <exception>

class HttpException : public std::exception {
	private:
		int _statusCode;
		std::string _message;

	public:
		HttpException(int code, const std::string& msg) : _statusCode(code), _message(msg) {}
		virtual ~HttpException() throw() {}

		virtual const char* what() const throw() {
			return _message.c_str();
		}

		int getStatusCode() const {
			return _statusCode;
		}
};