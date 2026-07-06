#pragma once

#include "SocketHandler.hpp"
#include <vector>
#include <poll.h>

class SocketManager
{
private:
	HandleSocket _listener;
	std::vector<HandleSocket*> _clients;
	std::vector<struct pollfd> _pollFds;

	void addToPoll(int fd);
	void removeFromPoll(size_t index);
	void handleNewConnection();
	void handleClientData(size_t pollIndex);
	void disconnectClient(size_t pollIndex);
public:
	SocketManager(const std::string &host, const int &port);
	~SocketManager();

	// Functs
	void setup();
	void run();
};
