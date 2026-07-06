#include "SocketHandler.hpp"

HandleSocket::HandleSocket() : _host(""), _port(-1), _fd(-1)
{
	std::memset(&_addr, 0, sizeof(_addr));
	_addrLen = 0;
}

HandleSocket::HandleSocket(const std::string &host, const int &port) :
	_host(host),
	_port(port),
	_fd(-1)
{
	std::memset(&_addr, 0, sizeof(_addr));
	_addrLen = 0;
}

HandleSocket::HandleSocket(int fd, const struct sockaddr_storage &addr, socklen_t addrLen) :
	_host(""),
	_port(-1),
	_fd(fd),
	_addr(addr),
	_addrLen(addrLen)
{
}

HandleSocket::HandleSocket(const HandleSocket &other) :
	_host(other._host),
	_port(other._port),
	_fd(other._fd),
	_addr(other._addr),
	_addrLen(other._addrLen)
{
}

HandleSocket::~HandleSocket()
{
	if (_fd != -1)
		close(_fd);
}

HandleSocket &HandleSocket::operator=(const HandleSocket &other)
{
	if (this != &other)
	{
		this->_host = other._host;
		this->_port = other._port;
		this->_fd  = other._fd;
		this->_addr = other._addr;
		this->_addrLen = other._addrLen;
	}
	return *this;
}

//Getters

int HandleSocket::getFD() const
{
	return this->_fd;
}

int HandleSocket::getPort() const
{
	return this->_port;
}

std::string HandleSocket::getHost() const
{
	return this->_host;
}

//Setters

void HandleSocket::setFD(const int &fd)
{
	this->_fd = fd;
}

void HandleSocket::setPort(const int &port)
{
	this->_port = port;
}

void HandleSocket::setHost(const std::string &host)
{
	this->_host = host;
}
