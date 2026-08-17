#include "SocketManager.hpp"
#include "Signals.hpp"

SocketManager::SocketManager() {}

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

			addPollFd(listener->getFD(), POLLIN);
			std::cout << "Listening " << addrs[j].host << ":" << addrs[j].port << " with FD " << listener->getFD() << std::endl;
		}

	}

}

// Gestion de fds por Fd

void SocketManager::addPollFd(int fd, short events)
{
	struct pollfd newPfd;
	newPfd.fd = fd;
	newPfd.events = events;
	newPfd.revents = 0;
	_pollFds.push_back(newPfd);
}

void SocketManager::removePollFd(int fd)
{
	for (size_t i = 0; i < _pollFds.size(); i++)
	{
		if (_pollFds[i].fd == fd)
		{
			_pollFds.erase(_pollFds.begin() + i);
			return;
		}
	}
}

void SocketManager::updatePollEvents(int fd, short events)
{
	for (size_t i = 0; i < _pollFds.size(); i++)
	{
		if (_pollFds[i].fd == fd)
		{
			_pollFds[i].events = events;
			return ;
		}
	}

}

// Loop Principal
void SocketManager::run()
{
	const int POLL_TIMEOUT_MS = 1000; // Timeout CGI

	while (!g_shutdown)
	{
		int ready = poll(&_pollFds[0], _pollFds.size(), POLL_TIMEOUT_MS);
		if (ready < 0)
		{
			if (errno == EINTR)
				continue;
			throw std::runtime_error("poll() failed");
		}
		for (size_t i = 0; i < _pollFds.size(); i++)
		{
			short revents = _pollFds[i].revents;
			if (revents == 0)
				continue;
			int fd = _pollFds[i].fd;
			if (_listenerConfig.find(fd) != _listenerConfig.end())
				handleNewConnection(fd);
			else if (_clientConfig.find(fd) != _clientConfig.end())
			{
				handleClientData(fd);
				break;
			}
			else if (_cgiFdToClient.find(fd) != _cgiFdToClient.end())
			{
				handleCgiEvent(fd, revents);
				break;
			}
		}
		checkAllCgiTimeouts();
	}
	std::cout << "\nBomb 💣" << std::endl;
}

// Nuevas Conexiones

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
	addPollFd(clientFd, POLLIN);
	_httpClients[clientFd] = new Http(*_clientConfig[clientFd]);
	std::cout << "New client with fd " << clientFd << std::endl;
}

//Gestionar D A T O S

void SocketManager::handleClientData(int fd)
{
	Http *http = _httpClients[fd];

	if (http->isWaitingOnCgi())
		return;

	char buffer[1024];
	ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);

	if (bytes <= 0)
	{
		disconnectClient(fd);
		return;
	}
	http->HttpRoutine(buffer, static_cast<size_t>(bytes));
	syncCgiState(fd, http);
}

//Sincronizando con el CGI Nueva York

void SocketManager::syncCgiState(int clientFd, Http *http)
{
	State status = http->getStatus();
	if (status == CGI_WRITING)
	{
		int wfd = http->getCgiWriteFd();
		if (wfd != -1 && _cgiFdToClient.find(wfd) == _cgiFdToClient.end())
		{
			addPollFd(wfd, POLLOUT);
			_cgiFdToClient[wfd] = clientFd;
		}
	}
	else if (status == CGI_READING)
	{
		int rfd = http->getCgiReadFd();
		if (rfd != -1 && _cgiFdToClient.find(rfd) == _cgiFdToClient.end())
		{
			addPollFd(rfd, POLLIN);
			_cgiFdToClient[rfd] = clientFd;
		}
	}
	else if (status == FINISHED)
		finishAndRespond(clientFd, http);
}

// Handelear Evento de la pipe del CGI

void SocketManager::handleCgiEvent(int fd, short revents)
{
	std::map<int, int>::iterator it = _cgiFdToClient.find(fd);
	if (it == _cgiFdToClient.end())
		return;
	int clientFd = it->second;
	Http *http = _httpClients[clientFd];

	if (revents & POLLOUT)
	{
		http->onCgiWritable();
		if (http->getCgiWriteFd() == -1)
		{
			removePollFd(fd);
			_cgiFdToClient.erase(fd);
			syncCgiState(clientFd, http);
		}
	}
	else if (revents & (POLLIN | POLLHUP | POLLERR))
	{
		http->onCgiReadable();
		if (http->getCgiReadFd() == -1)
		{
			removePollFd(fd);
			_cgiFdToClient.erase(fd);
			if (http->getStatus() == WRITING_RESPONSE)
				http->HttpRoutine(NULL, 0);
			if (http->getStatus() == FINISHED)
				finishAndRespond(clientFd, http);
		}
	}
}

// Todo donete ahora toca enviar el ojete

void SocketManager::finishAndRespond(int clientFd, Http *http)
{
	const Response &resp = http->getResponse();
	const std::vector<char> &raw = resp.getRawResponse();
	if (!raw.empty())
	{
		size_t totalSent = 0;
		while (totalSent < raw.size())
		{
			ssize_t sent = send(clientFd, &raw[0] + totalSent, raw.size() - totalSent, 0);
			if (sent <= 0)
				break;
			totalSent += sent;
		}
	}
	disconnectClient(clientFd);
}

//Ha morisionado el CGI nueva york?

void SocketManager::checkAllCgiTimeouts()
{
	const double CGI_TIMEOUT_SECONDS = 30.0;

	std::vector<int> toFinish;

	for (std::map<int, Http*>::iterator it = _httpClients.begin(); it != _httpClients.end(); ++it)
	{
		Http *http = it->second;
		if (http->isWaitingOnCgi() && http->checkCgiTimeout(CGI_TIMEOUT_SECONDS))
			toFinish.push_back(it->first);
	}
	for (size_t i = 0; i < toFinish.size(); i++)
	{
		int clientFd = toFinish[i];
		Http *http = _httpClients[clientFd];

		for (std::map<int, int>::iterator cit = _cgiFdToClient.begin(); cit != _cgiFdToClient.end();)
		{
			if (cit->second == clientFd)
			{
				removePollFd(cit->first);
				_cgiFdToClient.erase(cit++);
			}
			else
				++cit;

		}
		finishAndRespond(clientFd, http);
	}
}


// He disconnected
void SocketManager::disconnectClient(int fd)
{
	removePollFd(fd);
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
	std::cout << "Client disconnect with fd: " << fd << std::endl;

}

void SocketManager::resetRequest(int fd)
{
	delete _httpClients[fd];
	_httpClients[fd] = new Http(*_clientConfig[fd]);
}
