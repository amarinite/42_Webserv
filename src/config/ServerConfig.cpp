#include "ServerConfig.hpp"

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

	std::vector<Node*> locationNodes = getChildrenByType(serverRoot, NODE_BLOCK, "location");
	for (size_t i = 0; i < locationNodes.size(); i++)
		config._locations.push_back(LocationConfig::build(locationNodes[i]));

	std::vector<Node*> directiveNodes = getChildrenByType(serverRoot, NODE_DIR);
	for (size_t i = 0; i < directiveNodes.size(); ++i)
	{
		std::map<std::string, DirectiveHandler>::iterator it = handlers.find(directiveNodes[i]->name);
		if (it != handlers.end())
			(config.*(it->second))(directiveNodes[i]);
	}
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
	t_uri uri;
	parseUri(&uri, n->args[1]);

	_error_pages.push_back(std::make_pair(code, uri));
}

void ServerConfig::setClientMaxBodySize(const Node* n)
{
	int size = parseNumber(n->args[0]);
	_client_max_body_size = size;
}

void ServerConfig::setRoot(const Node* n)
{
	t_uri uri;
	parseUri(&uri, n->args[0]);

	_root = uri;
}

void ServerConfig::setIndex(const Node* n)
{
	_index = n->args[0];
}

const std::vector<ListenAddr>& ServerConfig::getListen() const
{
	return _listen;
}

const std::vector<std::pair<int, t_uri> >& ServerConfig::getErrorPages() const
{
	return _error_pages;
}

int ServerConfig::getClientMaxBodySize() const
{
	return _client_max_body_size;
}

const t_uri& ServerConfig::getRoot() const
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

const LocationConfig& ServerConfig::getLocationConfig(const t_uri& uri) const
{
	// ...
}