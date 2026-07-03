#include "SocketManager.hpp"

void HandleSocket::createSocket()
{
	struct addrinfo hints, *res;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	std::ostringstream oss;
	oss << _port;
	std::string portStr = oss.str();
	const char* hostPtr = _host.empty() ? NULL : _host.c_str();

	int status = getaddrinfo(hostPtr, portStr.c_str(), &hints, &res);
	if (status != 0)
		throw std::runtime_error(gai_strerror(status));
	_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (_fd < 0)
	{
		freeaddrinfo(res);
		throw std::runtime_error("socket() failed");
	}
	std::memcpy(&_addr, res->ai_addr, res->ai_addrlen);
	_addrLen = res->ai_addrlen;

	freeaddrinfo(res);
}

void HandleSocket::setReuseAddr()
{
	int opt = 1;
	if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		throw std::runtime_error("setsockopt() failed");
}

void HandleSocket::bindSocket()
{
	if (bind(_fd, (struct sockaddr*)&_addr, _addrLen) < 0)
		throw std::runtime_error("bind() failed");
}

void HandleSocket::listenSocket()
{
	if (listen(_fd, SOMAXCONN) < 0)
		throw std::runtime_error("listen() failed");
}

void HandleSocket::setNonBlocking()
{
	int flags = fcntl(_fd, F_GETFL, 0);
	if (flags < 0)
		throw std::runtime_error("fcntl(GETFL) failed");

	if (fcntl(_fd, F_SETFL, flags | O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl(SETFL) failed");
}
