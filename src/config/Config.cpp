#include "Config.hpp"

Config Config::build(Node* root)
{
	Config config;
	std::vector<Node*> serverNodes = getChildrenByType(root, NODE_BLOCK, "server");

	for (size_t i = 0; i < serverNodes.size(); i++)
		config._servers.push_back(ServerConfig::build(serverNodes[i]));
	return config;
}

const std::vector<ServerConfig>& Config::getServers() const
{
	return _servers;
}

const ServerConfig& Config::getServer(std::pair<std::string, int> ip_port) const
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		if (ip_port == _servers[i].getListen())
			return _servers[i];
	}
	return NULL;
}