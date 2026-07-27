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
		const std::vector<ListenAddr>& addrs = _servers[i].getListenVector();
		for (size_t j = 0; j < addrs.size(); j++)
		{
			if (ipPort == addrs[j])
				return _servers[i];
		}
	}

	// IS THIS POSSIBLE? PREGUNTAR ISAAC
	// No se (?
	throw std::runtime_error("Config::getServer: no server for given ip:port");
}
