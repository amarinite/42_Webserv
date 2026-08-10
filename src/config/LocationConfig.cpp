#include "LocationConfig.hpp"
#include "ServerConfig.hpp"

LocationConfig::LocationConfig() : _autoindex(false), _index()
{
	_index.push_back("index.html");
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

LocationConfig LocationConfig::buildFrom(Node* locationNode, const std::string& root, const std::vector<std::string>& index)
{
	static std::map<std::string, DirectiveHandler> handlers = initHandlers();
	LocationConfig config;
	config._root = root;
	config._index = index;

	t_uri uri;
	parseUri(uri, locationNode->args[0]);
	config._path = uri;

	std::vector<Node*> directiveNodes = getChildrenByType(locationNode, NODE_DIR);
	for (size_t i = 0; i < directiveNodes.size(); ++i)
	{
		std::map<std::string, DirectiveHandler>::iterator it = handlers.find(directiveNodes[i]->name);
		if (it != handlers.end())
			(config.*(it->second))(directiveNodes[i]);
	}

	std::vector<Node*> nestedNodes = getChildrenByType(locationNode, NODE_BLOCK, "location");
	for (size_t i = 0; i < nestedNodes.size(); ++i)
		config._locations.push_back(LocationConfig::build(nestedNodes[i], config));

	return config;
}

LocationConfig LocationConfig::build(Node* locationNode, const ServerConfig& parent)
{
	return buildFrom(locationNode, parent.getRoot(), parent.getIndex());
}

LocationConfig LocationConfig::build(Node* locationNode, const LocationConfig& parent)
{
	return buildFrom(locationNode, parent.getRoot(), parent.getIndex());
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
	_upload_store = n->args[0];
}

void LocationConfig::setCgiExtension(const Node* n)
{
	std::string ext = n->args[0];

	_cgi_extension[ext] = n->args[1];
}

void LocationConfig::setRoot(const Node* n)
{
	_root = parseFsPath(n->args[0]);
}

void LocationConfig::setIndex(const Node* n)
{
	_index.clear();
	for (size_t i = 0; i < n->args.size(); i++)
		_index.push_back(n->args[i]);
}

bool LocationConfig::hasAutoIndex() const
{
	return _autoindex;
}

bool LocationConfig::hasUploadEnabled() const
{
	return !_upload_store.empty();
}

bool LocationConfig::hasCgi() const
{
	return !_cgi_extension.empty();
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

const std::string& LocationConfig::getUploadStore() const
{
	return _upload_store;
}

const std::map<std::string, std::string>& LocationConfig::getCgiExtension() const
{
	return _cgi_extension;
}

const std::string& LocationConfig::getRoot() const
{
	return _root;
}

const std::vector<std::string>& LocationConfig::getIndex() const
{
	return _index;
}

const std::vector<LocationConfig>& LocationConfig::getLocations() const
{
	return _locations;
}

const LocationConfig* LocationConfig::getLocationConfig(const t_uri& uri) const
{
	const LocationConfig* best = NULL;
	size_t bestLen = 0;
	for (size_t i = 0; i < _locations.size(); ++i)
	{
		const std::string& path = _locations[i].getPath().path;
		if (isValidMatch(uri.path, path) && path.size() > bestLen)
		{
			best = &_locations[i];
			bestLen = path.size();
		}
	}
	if (!best)
		return NULL;
	
	const LocationConfig* deeper = best->getLocationConfig(uri);
	return deeper ? deeper : best;
}