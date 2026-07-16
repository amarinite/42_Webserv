#include "ServerConfig.hpp"

ServerConfig::ServerConfig()
	: _client_max_body_size(1000000), _index("index.html")
{
}

std::map<std::string, ServerConfig::DirectiveHandler> ServerConfig::initHandlers()
{
	std::map<std::string, DirectiveHandler> m;
	m["listen"]					= &ServerConfig::setListen;
	m["error_page"]				= &ServerConfig::setErrorPage;
	m["client_max_body_size"]	= &ServerConfig::setClientMaxBodySize;
	m["root"]					= &ServerConfig::setRoot;
	m["index"]					= &ServerConfig::setIndex;
	return m;
}

ServerConfig ServerConfig::build(Node* serverRoot)
{
	static std::map<std::string, DirectiveHandler> handlers = initHandlers();
	ServerConfig config;

	std::vector<Node*> directiveNodes = getChildrenByType(serverRoot, NODE_DIR);
	for (size_t i = 0; i < directiveNodes.size(); ++i)
	{
		std::map<std::string, DirectiveHandler>::iterator it = handlers.find(directiveNodes[i]->name);
		if (it != handlers.end())
			(config.*(it->second))(directiveNodes[i]);
	}

	bool hasRootLocation = false;
	std::vector<Node*> locationNodes = getChildrenByType(serverRoot, NODE_BLOCK, "location");
	for (size_t i = 0; i < locationNodes.size(); i++)
	{
		LocationConfig loc = LocationConfig::build(locationNodes[i], config);
		if (loc.getPath().path == "/")
			hasRootLocation = true;
		config._locations.push_back(loc);
	}

	if (!hasRootLocation)
		config._locations.push_back(LocationConfig::buildDefault(config));

	return config;
}

void ServerConfig::setListen(const Node* n)
{
	for (size_t i = 0; i < n->args.size(); i++)
	{
		ListenAddr addr = parseListenArg(n->args[i]);
		_listen.push_back(addr);
	}
}

void ServerConfig::setErrorPage(const Node* n)
{
	int code = parseNumber(n->args[0]);

	_error_pages[code] = n->args[1];
}

void ServerConfig::setClientMaxBodySize(const Node* n)
{
	int size = parseNumber(n->args[0]);
	_client_max_body_size = size;
}

void ServerConfig::setRoot(const Node* n)
{
	_root = n->args[0];
}

void ServerConfig::setIndex(const Node* n)
{
	_index = n->args[0];
}

const std::vector<ListenAddr>& ServerConfig::getListen() const
{
	return _listen;
}

const std::map<int, std::string>& ServerConfig::getErrorPages() const
{
	return _error_pages;
}

int ServerConfig::getClientMaxBodySize() const
{
	return _client_max_body_size;
}

const std::string& ServerConfig::getRoot() const
{
	return _root;
}

const std::string& ServerConfig::getIndex() const
{
	return _index;
}

const std::vector<LocationConfig>& ServerConfig::getLocations() const
{
	return _locations;
}

bool isValidMatch(const std::string& reqPath, const std::string& configPath)
{
	if (reqPath.compare(0, configPath.size(), configPath) != 0)
		return false;

	if (reqPath.size() == configPath.size())
		return true;

	if (configPath[configPath.size() - 1] == '/')
		return true;

	if (reqPath[configPath.size()] == '/')
		return true;

	return false;
}

const LocationConfig& ServerConfig::getLocationConfig(const t_uri& uri) const
{
	const LocationConfig* best = NULL;
	size_t bestLen = 0;

	for (size_t i = 0; i < _locations.size(); ++i)
	{
		const std::string& locPath = _locations[i].getPath().path;
		if (isValidMatch(uri.path, locPath) && locPath.size() > bestLen)
		{
			best = &_locations[i];
			bestLen = locPath.size();
		}
	}
	return *best;
}