#pragma once
#include <string>
#include <map>
#include <unistd.h> 
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <new> 
#include "ServerConfig.hpp"
#include "LocationConfig.hpp"
#include "HttpRequest.hpp"
#include "HttpException.hpp"
#include "ParseUtils.hpp"
#include "FileUtils.hpp"
#include "UriParser.hpp"

class CgiHandler
{
	private:
		void			validateCgiPaths(const std::string& scriptPath, const std::string& execPath);
		char**			envArrayFromMap(const std::map<std::string, std::string>& env);

	public:
		static bool		canHandleCgi(const t_uri& uri, const LocationConfig& conf);
		static char**	buildCgiEnv(const Request& req, const ServerConfig& conf, const std::string& path, const std::string& ip);
		static void		freeCgiEnv(char** envp);
};