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
	for (std::map<int, Request*>::iterator it = _requests.begin(); it != _requests.end(); ++it)
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
	_requests[clientFd] = new Request(_clientConfig[clientFd]->getClientMaxBodySize());
	std::cout << "Nuevos cliente, fd " << clientFd << std::endl;
}

void SocketManager::handleClientData(size_t pollIndex)
{
	int fd = _pollFds[pollIndex].fd;
	char buffer[1024];
	ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);
	if (bytes <= 0)
	{
		disconnectClient(pollIndex);
		return;
	}
	Request *req = _requests[fd];
	req->setStream(req->getLeftover() + std::string(buffer, bytes));
	req->clearLeftover();
	try
	{
		while (true)
		{
			if (!req->parseRequestHead())
				break;
			if (!req->parseRequestBody())
				break;
			std::cout << "--- Request completa de fd " << fd
					<< " metodo: " << req->getMethod() << " ---" << std::endl;
			std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nOK";
			send(fd, response.c_str(), response.size(), 0);
			std::string leftover = req->getLeftover();
			resetRequest(fd);
			req = _requests[fd];
			req->setStream(leftover);
			if (leftover.empty())
				break;
		}
	}
	catch(const HttpException &e)
	{
		std::ostringstream oss;
		std::string body = e.what();
		oss << "HTTP/1.1 " << e.getStatusCode() << " Error\r\nContent-Length: "
            << body.size() << "\r\n\r\n" << body;
		send (fd, oss.str().c_str(), oss.str().size(), 0);
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
	delete _requests[fd];
	_requests.erase(fd);
	_clientConfig.erase(fd);
	_pollFds.erase(_pollFds.begin() + pollIndex);

	std::cout << "Cliente desconectado, fd: " << fd << std::endl;

}

void SocketManager::resetRequest(int fd)
{
	delete _requests[fd];
	_requests[fd] = new Request(_clientConfig[fd]->getClientMaxBodySize());
}
