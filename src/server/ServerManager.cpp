#include "ServerManager.hpp"

ServerManager::ServerManager(const Config &config) : _config(config)
{
}

ServerManager::~ServerManager()
{
}

void ServerManager::run()
{
	std::cout << "ServerManager running with " << _config.getServers().size()
	<< " server block(s)." << std::endl;

	_socketManager.setup(_config.getServers());
	_socketManager.run();
}

const std::vector<ServerConfig> &ServerManager::getServers() const
{
	return _config.getServers();
}
