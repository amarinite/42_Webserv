#pragma once

#include "Config.hpp"
#include "ServerConfig.hpp"
#include "SocketManager.hpp"
#include <vector>
#include <iostream>

class ServerManager
{
private:
	Config _config;
	SocketManager _socketManager;

	ServerManager(const ServerManager &other);
	ServerManager &operator=(const ServerManager &other);
public:
	ServerManager(const Config &config);
	~ServerManager();

	void run();

	const std::vector <ServerConfig> &getServers() const;
};
