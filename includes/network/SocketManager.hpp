#pragma once

#include "SocketHandler.hpp"
#include "HttpRequest.hpp"
#include "HttpException.hpp"
#include "ServerConfig.hpp"
#include "Http.hpp"
#include <vector>
#include <map>
#include <poll.h>
#include <stdexcept>
#include <iostream>
#include <cstring>

class SocketManager
{
private:
	std::vector<HandleSocket*>		_listeners;
	std::vector<HandleSocket*>		_clients;
	std::vector<struct pollfd>		_pollFds;
	std::map<int, Http*>			_httpClients;
	std::map<int, const ServerConfig*>	_listenerConfig;
	std::map<int, const ServerConfig*>	_clientConfig;
	std::map<int, int>					_cgiFdToClient;

	void addPollFd(int fd, short events);
	void removePollFd(int fd);
	void updatePollEvents(int fd, short events);

	void handleNewConnection(int listenerFd);
	void handleClientData(int fd);
	void handleCgiEvent(int fd, short revents);
	void disconnectClient(int fd);
	void resetRequest(int fd);

	void syncCgiState(int clientFd, Http *http);
	void finishAndRespond(int clientFd, Http *http);
	void checkAllCgiTimeouts();

	SocketManager(const SocketManager &other);
	SocketManager &operator=(const SocketManager &other);
public:
	SocketManager();
	~SocketManager();

	// Functs
	void setup(const std::vector<ServerConfig> &configs);
	void run();
};
