#include "CgiHandler.hpp"

void CgiHandler::validateCgiPaths(const std::string& scriptPath, const std::string& execPath)
{
	// Script file does not exist
	if (access(scriptPath.c_str(), F_OK) != 0)
		throw HttpException(404, "Script not found");

	// Script file does not have permissions
	if (access(scriptPath.c_str(), R_OK | X_OK) != 0)
		throw HttpException(403, "Script not executable");

	// Interpreter program does not exist
	if (access(execPath.c_str(), F_OK) != 0)
		throw HttpException(500, "Interpreter not found");

	// Interpreter program does not have permissions
	if (access(execPath.c_str(), X_OK) != 0)
		throw HttpException(500, "Interpreter not executable");
}

bool CgiHandler::canHandleCgi(const t_uri& uri, const LocationConfig& conf)
{
	// No cgi_extension directive configured in the matching location block.
	if (!conf.hasCgi())
		return false;

	std::string fileExt = findFileExtension(uri.path);
	// Path is a directory: A request for /cgi-bin/ (where your server might check for an index directive or autoindex instead).
	if (fileExt.empty())
		return false;

	// Extension mismatch: The location has cgi_extension .py, but the request is for /cgi-bin/logo.png or /index.html.
	const std::map<std::string, std::string>& cgiExt = conf.getCgiExtension();
	if (cgiExt.count(fileExt) == 0)
		return false;

	std::string scriptPath = conf.getRoot() + uri.path;
	std::map<std::string, std::string>::const_iterator it = cgiExt.find(fileExt);
	if (it != cgiExt.end())
		validateCgiPaths(scriptPath, it->second);

	return true;
}

char** CgiHandler::buildCgiEnv(const Request& req, const ServerConfig& conf, const std::string& ip)
{
	std::map<std::string, std::string> env;
	const std::vector<ListenAddr>& addrs = conf.getListenVector();
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["SERVER_SOFTWARE"] = "Group de Afectadous by Taha";
	env["SERVER_NAME"] = addrs.empty() ? "" : addrs[0].host;// Get it from servConf
	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_PORT"] = numberToString(addrs.empty() ? 0 : addrs[0].port); // Get it from servConf
	env["REQUEST_METHOD"] = req.getMethod();
	env["SCRIPT_NAME"] = req.getUri().path;
	env["REMOTE_ADDR"] = ip; // IP CLIENT
	env["QUERY_STRING"] = req.getUri().query;
	// path info

	if (!req.getBody().empty())
	{
		env["CONTENT_LENGTH"] = numberToString(req.getBody().size());
		std::map<std::string, std::string>::const_iterator it = req.getHeaders().find("content-type");
		env["CONTENT_TYPE"] = it != req.getHeaders().end() ? it->second : "application/octet-stream";
	}

	// build headers
	std::map<std::string, std::string>::const_iterator hit;
	for (hit = req.getHeaders().begin(); hit != req.getHeaders().end(); hit++)
	{
		std::string name = hit->first;
		for (size_t i = 0; i < name.size(); ++i)
		{
			if (name[i] == '-')
				name[i] = '_';
			else
				name[i] = std::toupper(static_cast<unsigned char>(name[i]));
		}

		if (name == "CONTENT_TYPE" || name == "CONTENT_LENGTH")
			continue;
		env["HTTP_" + name] = hit->second;
	}

	return envArrayFromMap(env);
}

char** CgiHandler::envArrayFromMap(const std::map<std::string, std::string>& env)
{
	char** envp = static_cast<char**>(malloc(sizeof(char*) * (env.size() + 1)));
	if (!envp)
		throw std::bad_alloc();

	size_t i = 0;
	std::map<std::string, std::string>::const_iterator it;
	for (it = env.begin(); it != env.end(); ++it, ++i)
	{
		std::string entry = it->first + "=" + it->second;

		envp[i] = static_cast<char*>(malloc(entry.size() + 1));
		if (!envp[i])
		{
			for (size_t j = 0; j < i; ++j)
				free(envp[j]);
			free(envp);
			throw std::bad_alloc();
		}
		std::strcpy(envp[i], entry.c_str());
	}
	envp[env.size()] = NULL;

	return envp;
}

void CgiHandler::freeCgiEnv(char** envp)
{
	if (!envp)
		return;

	for (size_t i = 0; envp[i] != NULL; ++i)
		free(envp[i]);
	free(envp);
}
