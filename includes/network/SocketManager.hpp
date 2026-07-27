#pragma once

#include "SocketHandler.hpp"
#include "HttpRequest.hpp"
#include "HttpException.hpp"
#include "ServerConfig.hpp"
#include <vector>
#include <map>
#include <poll.h>

class SocketManager
{
private:
	std::vector<HandleSocket*>		_listeners;
	std::vector<HandleSocket*>		_clients;
	std::vector<struct pollfd>		_pollFds;
	std::map<int, Request*>			_requests;
	std::map<int, const ServerConfig*>	_listenerConfig;
	std::map<int, const ServerConfig*>	_clientConfig;

	void addToPoll(int fd);
	void handleNewConnection(int listenerFd);
	void handleClientData(size_t pollIndex);
	void disconnectClient(size_t pollIndex);
	void resetRequest(int fd);

	SocketManager(const SocketManager &other);
	SocketManager &operator=(const SocketManager &other);
public:
	SocketManager();
	~SocketManager();

	// Functs
	void setup(const std::vector<ServerConfig> &configs);
	void run();
};
