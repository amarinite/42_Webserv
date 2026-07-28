#include "SocketManager.hpp"

SocketManager::SocketManager()
{
}

SocketManager::~SocketManager()
{
	for (size_t i = 0; i < _listeners.size(); i++)
		delete _listeners[i];
	for (size_t i = 0; i < _clients.size(); i++)
		delete _clients[i];
	for (std::map<int, Http*>::iterator it = _httpClients.begin(); it != _httpClients.end(); ++it)
		delete it->second;
}

void SocketManager::setup(const std::vector<ServerConfig> &configs)
{
	for (size_t i = 0; i < configs.size(); i++)
	{
		const std::vector <ListenAddr> &addrs = configs[i].getListenVector();

		for (size_t j = 0; j < addrs.size(); j++)
		{
			HandleSocket *listener = new HandleSocket(addrs[j].host, addrs[j].port);
			listener->createSocket();
			listener->setReuseAddr();
			listener->bindSocket();
			listener->listenSocket();
			listener->setNonBlocking();

			_listeners.push_back(listener);
			_listenerConfig[listener->getFD()] = &configs[i];

			addToPoll(listener->getFD());
			std::cout << "Escuchando " << addrs[j].host << ":" << addrs[j].port << " con fd " << listener->getFD() << std::endl;
		}

	}

}

void SocketManager::run()
{
	// CONTROLAR CGI:
	// for writing (POLLOUT) -> Call CgiExecutor::handleWriteEvent()
	// for reading (POLLIN) -> Call CgiExecutor::handleReadEvent()
	// Check for both POLLIN and POLLOUT, and route events based on what type of file descriptor
	// _pollFds[i].fd actually is (listener socket, client socket, CGI read pipe, or CGI write pipe)
	while (true)
	{
		int ready = poll(&_pollFds[0], _pollFds.size(), -1);
		if (ready < 0)
			throw std::runtime_error("poll() failed");
		for (size_t i = 0; i < _pollFds.size(); i++)
		{
			if (!(_pollFds[i].revents & POLLIN))
				continue;
			int fd = _pollFds[i].fd;
			if (_listenerConfig.find(fd) != _listenerConfig.end())
				handleNewConnection(fd);
			else
			{
				handleClientData(i);
				break;
			}
		}
	}
}

void SocketManager::addToPoll(int fd)
{
	struct pollfd newPfd;
	newPfd.fd = fd;
	newPfd.events = POLLIN;
	newPfd.revents = 0;
	_pollFds.push_back(newPfd);
}

void SocketManager::handleNewConnection(int listenerFd)
{
	struct sockaddr_storage clientAddr;
	socklen_t addrLen = sizeof(clientAddr);

	int clientFd = accept(listenerFd, (struct sockaddr*)&clientAddr, &addrLen);
	if (clientFd < 0)
		return;

	HandleSocket *client = new HandleSocket(clientFd, clientAddr, addrLen);
	client->setNonBlocking();

	_clients.push_back(client);
	_clientConfig[clientFd] = _listenerConfig[listenerFd];
	addToPoll(clientFd);
	_httpClients[clientFd] = new Http(*_clientConfig[clientFd]);
	std::cout << "Nuevos cliente, fd " << clientFd << std::endl;
}

void SocketManager::handleClientData(size_t pollIndex)
{
	int fd = _pollFds[pollIndex].fd;
	std::cout << "[DEBUG] handleClientData llamado para fd " << fd << std::endl;
	char buffer[1024];
	ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);
	std::cout << "[DEBUG] recv() devolvio " << bytes << " bytes" << std::endl;
	if (bytes <= 0)
	{
		disconnectClient(pollIndex);
		return;
	}
	Http *http = _httpClients[fd];
	http->HttpRoutine(buffer, static_cast<size_t>(bytes));
	std::cout << "[DEBUG] Status tras HttpRoutine: " << http->getStatus() << std::endl;
	if (http->getStatus() == FINISHED)
	{
		std::cout << "[DEBUG] Request FINISHED, enviando respuesta" << std::endl;
		const Response &resp = http->getResponse();
		const std::vector<char> &raw = resp.getRawResponse();
		std::cout << "[DEBUG] Tamano de la respuesta: " << raw.size() << std::endl;
		if (!raw.empty())
			sendAll(fd, &raw[0], raw.size());
		disconnectClient(pollIndex);
	}

}

void SocketManager::disconnectClient(size_t pollIndex)
{
	int fd = _pollFds[pollIndex].fd;

	for (size_t i = 0; i < _clients.size(); i++)
	{
		if (_clients[i]->getFD() == fd)
		{
			delete _clients[i];
			_clients.erase(_clients.begin() + i);
			break;
		}
	}
	delete _httpClients[fd];
	_httpClients.erase(fd);
	_clientConfig.erase(fd);
	_pollFds.erase(_pollFds.begin() + pollIndex);

	std::cout << "Cliente desconectado, fd: " << fd << std::endl;

}

void SocketManager::resetRequest(int fd)
{
	delete _httpClients[fd];
	_httpClients[fd] = new Http(*_clientConfig[fd]);
}

void SocketManager::sendAll(int fd, const char *data, size_t len)
{
	size_t totalSent = 0;
	while (totalSent < len)
	{
		ssize_t sent = send(fd, data + totalSent, len - totalSent, 0);
		if (sent <= 0)
			break;
		totalSent += sent;
	}

}
