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
#include "HttpException.hpp"

struct t_uri {
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
};

void			parseUri(t_uri &uri, std::string req);
std::string		toString(t_uri &uri);
