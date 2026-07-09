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

const ServerConfig& Config::getServer(const ListenAddr& ipPort) const
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		for (size_t j = 0; j < _servers[i].getListen().size(); j++)
		{
			if (ipPort == _servers[i].getListen()[j])
				return _servers[i];
		}
	}

	// IS THIS POSSIBLE? PREGUNTAR ISAAC
	throw std::runtime_error("Config::getServer: no server for given ip:port");
}