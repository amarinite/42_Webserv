#include "UriParser.hpp"

static const std::string SCHEME_DEL	= ":";
static const std::string AUTH_DEL	= "/?#";
static const std::string PATH_DEL	= "?#";
static const std::string QUERY_DEL	= "#";

static std::string extractUntil(std::string &line, const std::string &delimiter) {
	std::string extracted;

	if (line.empty()) 
		return "";

	size_t pos = line.find_first_of(delimiter);
	if (pos == std::string::npos) {
		extracted = line;
		line.clear();
		return extracted;
	}

	extracted = line.substr(0, pos);
	line.erase(0, pos + 1);
	return extracted;
}

static void isHost(std::string &req) {
	if (req.compare(0, 2, "//") == 0)
		req.erase(0, 2);
}

static int schemeCheck(t_uri &uri) {
	for (size_t i = 0; i < uri.scheme.length(); ++i) {
		uri.scheme[i] = std::tolower(uri.scheme[i]);
	}
	if (uri.scheme == "http")
		return 0;
	return 1;
}

static int authCheck(t_uri &uri) {
	if (uri.authority.find('@') != std::string::npos)
		return 1;

	std::string host;
	std::string port;

	size_t port_pos = uri.authority.find(':');
	if (port_pos == std::string::npos) {
		host = uri.authority;
	} else {
		if (uri.authority.find(':', port_pos + 1) != std::string::npos)
			return 1;

		host = uri.authority.substr(0, port_pos);
		port = uri.authority.substr(port_pos + 1);

		if (port.empty() || port.size() > 5)
			return 1;

		for (size_t i = 0; i < port.size(); ++i) {
			if (!std::isdigit(port[i]))
				return 1;
		}

		std::stringstream ss(port);
		long port_val;
		ss >> port_val;
		if (port_val > 65535 || port_val < 0)
			return 1;
	}

	for (size_t i = 0; i < host.size(); ++i) {
		unsigned char c = static_cast<char>(host[i]);
		host[i] = static_cast<char>(std::tolower(c));
	}

	struct addrinfo hints;
	struct addrinfo *res;

	std::memset(&hints, 0 , sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV;

	const char *port_cstr = port.empty() ? "80" : port.c_str();

	int gai = getaddrinfo(host.c_str(), port_cstr, &hints, &res);
	if (gai != 0)
		return 1;
	freeaddrinfo(res);
	if (!port.empty()) {
		std::stringstream ss;
		ss << port;
		ss >> uri.auth_port;
	} else {
		uri.auth_port = 80;
	}
	uri.authority.clear();
	uri.authority = host;

	return 0;
}

static int vectorizePath(t_uri &uri, std::vector<std::string> &route) {
	if (uri.path.empty() || uri.path == "/") {
		route.push_back("/");
		return 0;
	}
	
	std::stringstream ss(uri.path);
	std::string segment;

	while (std::getline(ss, segment, '/')) {
		if (segment.empty() || !segment.empty()) {
			if (segment == ".")
                continue;
			if (segment == "..") {
				if (!route.empty()) {
					route.pop_back();
					continue;
				} else {
					return 1;
				}
			}
			route.push_back(segment);
		}
	}
	return 0;
}

static void rebuildPath(t_uri &uri, const std::vector<std::string> &route) {
	if (route.empty()) {
        uri.path = "/";
        return;
    }

	std::string new_path = "";
    for (size_t i = 0; i < route.size(); ++i) {
		if (route[i].empty())
			++i;
		new_path += "/" + route[i];
    }
    uri.path = new_path;
}

static int pathCheck(t_uri &uri) {
	if (uri.path.find(' ') != std::string::npos
		|| uri.path.find('{') != std::string::npos
		|| uri.path.find('}') != std::string::npos)	
		return 1;

	if (uri.path == "/")
		return 0;
	std::vector<std::string> route;
	if (vectorizePath(uri, route))
		return 1;
	rebuildPath(uri, route);
	return 0;
}

static void detectQueryFrag(t_uri &uri, std::string req, size_t checked_pos) {
	if (checked_pos == std::string::npos) {
		checked_pos = 0;
	}

	size_t q_pos = req.find('?');
	size_t f_pos = req.find('#');

	if (f_pos != std::string::npos && f_pos > checked_pos)
		uri.has_frag = true;
	else
		uri.has_frag = false;

	if (q_pos != std::string::npos && q_pos > checked_pos) {
		if (uri.has_frag == false || q_pos < f_pos) {
			uri.has_query = true;
		} else {
			uri.has_query = false;
		}
	}
}

static int detectPath(t_uri &uri, std::string req) {
	size_t path_begin = req.find('/');
	if (uri.has_auth == false) {
		if (path_begin == 0) {
			uri.has_path = true;
			return 0;
		} else {
			return 1;
		}
	}
	if (uri.has_auth == true) {
		if (path_begin != std::string::npos) {
			uri.has_path = true;
		} else {
			uri.has_path = false;
		}
		return 0;
	}
	return 1;
}

static int isDirectPath(t_uri &uri, std::string req) {
	if (req.find("://") != std::string::npos) {
        return 0;
    }
	
	if (req[0] == '/' && req.size() == 1) {
		uri.scheme = '/';
		return 0;
	}
	return 1;
}

static int detectDelimiters(t_uri &uri, std::string req) {
	if (req.empty()
		|| req.find('[') != std::string::npos
		|| req.find(']') != std::string::npos)
		return 1;

	size_t checked_pos;
	if (isDirectPath(uri, req)) {
		uri.has_scheme = false;
		uri.has_auth = false;
	} else {
		uri.has_scheme = true;
		checked_pos = req.find(':');
		if (checked_pos != std::string::npos
			&& req.compare(checked_pos + 1, 2, "//") == 0) {
			uri.has_auth = true;
		} else {
			return 1;
		}
	}
	if (detectPath(uri, req)) {
		return 1;
	} else {
		checked_pos = req.find_first_of('/');
	}
	detectQueryFrag(uri, req, checked_pos);
	return 0;
}

static void init_struct(t_uri &uri) {
	uri.has_scheme = false;
    uri.has_auth = false;
    uri.has_path = false;
    uri.has_query = false;
    uri.has_frag = false;
    uri.auth_port = 0;
}

void	parseUri(t_uri &uri, std::string req) {
	init_struct(uri);
	if (detectDelimiters(uri, req))
		throw HttpException(400, "Bad Request: Invalid Delimiters");
	if (uri.has_scheme) {
		uri.scheme = extractUntil(req, SCHEME_DEL);
		if (schemeCheck(uri))
			throw HttpException(400, "Bad Request: Invalid Delimiters");
		
		isHost(req);
		if (uri.has_path || uri.has_query || uri.has_frag)
			uri.authority = extractUntil(req, AUTH_DEL);
		else {
			uri.authority = req;
			req.clear();
		}
		uri.path = -1;
		if (authCheck(uri))
			throw HttpException(400, "Bad Request: Invalid Delimiters");
	}
	if (uri.has_path) {
		if (uri.has_query || uri.has_frag) {
			uri.path = extractUntil(req, PATH_DEL);
		}
		else {
			uri.path = req;
			req.clear();
		}
		if (pathCheck(uri))
			throw HttpException(400, "Bad Request: Invalid Delimiters");
	}
	if (uri.has_query) {
		if (uri.has_frag)
			uri.query = extractUntil(req, QUERY_DEL);
		else {
			uri.query = req;
			req.clear();	
		}
	}
	if (uri.has_frag) {
		uri.fragment = req;
		req.clear();
	}
}