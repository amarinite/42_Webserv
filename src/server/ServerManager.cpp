#include "ServerManager.hpp"

ServerManager::ServerManager(const Config &config) : _config(config)
{
}

ServerManager::~ServerManager()
{
}

void ServerManager::run()
{
	std::cout << "ServerManager corriendo con " << _config.getServers().size()
	<< " server block(s)." << std::endl;

	//Inserta llamadas al SocketManager para cada instancia
}

const std::vector<ServerConfig> &ServerManager::getServers() const
{
	return _config.getServers();
}
