#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

LocationConfig::LocationConfig()
	: _autoindex(false), _index("index.html")
{
	_allowed_methods.push_back("GET");
}

std::map<std::string, LocationConfig::DirectiveHandler> LocationConfig::initHandlers()
{
	std::map<std::string, DirectiveHandler> m;
	m["allowed_methods"]	= &LocationConfig::setAllowedMethods;
	m["redirect"]			= &LocationConfig::setRedirect;
	m["autoindex"]			= &LocationConfig::setAutoindex;
	m["upload_store"]		= &LocationConfig::setUploadStore;
	m["cgi_extension"]		= &LocationConfig::setCgiExtension;
	m["root"]				= &LocationConfig::setRoot;
	m["index"]				= &LocationConfig::setIndex;
	return m;
}

LocationConfig LocationConfig::build(Node* locationNode, const ServerConfig& parent)
{
	LocationConfig config;

	config._root = parent.getRoot();
	config._index = parent.getIndex();

	static std::map<std::string, DirectiveHandler> handlers = initHandlers();

	t_uri uri;
	parseUri(uri, locationNode->args[0]);
	config._path = uri;

	std::vector<Node *> directiveNodes = getChildrenByType(locationNode, NODE_DIR);
	for (size_t i = 0; i < directiveNodes.size(); i++)
	{
		std::map<std::string, DirectiveHandler>::iterator it = handlers.find(directiveNodes[i]->name);
		if (it != handlers.end())
			(config.*(it->second))(directiveNodes[i]);
	}

	// TO DO: handle nested locations

	return config;
}

LocationConfig LocationConfig::buildDefault(const ServerConfig& parent)
{
	LocationConfig config;
	t_uri uri;
	parseUri(uri, "/");

	config._path = uri;
	config._root = parent.getRoot();
	config._index = parent.getIndex();

	return config;
}

void LocationConfig::setAllowedMethods(const Node* n)
{
	_allowed_methods.clear();
	for (size_t i = 0; i < n->args.size(); i++)
		_allowed_methods.push_back(n->args[i]);
}

void LocationConfig::setRedirect(const Node* n)
{
	t_uri uri;
	parseUri(uri, n->args[0]);

	_redirect = uri;
}

void LocationConfig::setAutoindex(const Node* n)
{
	_autoindex = n->args[0] == "on" ? true : false;
}

void LocationConfig::setUploadStore(const Node* n)
{
	t_uri uri;
	parseUri(uri, n->args[0]);

	_upload_store = uri;
}

void LocationConfig::setCgiExtension(const Node* n)
{
	std::string ext = n->args[0];
	t_uri uri;
	parseUri(uri, n->args[1]);

	_cgi_extension[ext] = uri;
}

void LocationConfig::setRoot(const Node* n)
{
	t_uri uri;
	parseUri(uri, n->args[0]);

	_root = uri;
}

void LocationConfig::setIndex(const Node* n)
{
	_index = n->args[0];
}

bool LocationConfig::hasAutoindex() const
{
	return _autoindex;
}

bool LocationConfig::hasUploadEnabled() const
{
	return _upload_store.has_path;
}

bool LocationConfig::hasCgi() const
{
	return !_cgi_extension.empty();
}

const t_uri& LocationConfig::getPath() const
{
	return _path;
}

const std::vector<std::string>& LocationConfig::getAllowedMethods() const
{
	return _allowed_methods;
}

const t_uri& LocationConfig::getRedirect() const
{
	return _redirect;
}

const t_uri& LocationConfig::getUploadStore() const
{
	return _upload_store;
}

const std::map<std::string, t_uri>& LocationConfig::getCgiExtension() const
{
	return _cgi_extension;
}

const t_uri& LocationConfig::getRoot() const
{
	return _root;
}

const std::string& LocationConfig::getIndex() const
{
	return _index;
}