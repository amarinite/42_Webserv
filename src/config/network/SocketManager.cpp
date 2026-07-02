#include "SocketManager.hpp"

HandleSocket::HandleSocket()
{
	std::cout << "Como hemos llegado aqui?" << std::endl;
}

HandleSocket::HandleSocket(const std::string &host, const int &port) :
	_host(host),
	_port(port),
	_fd(-1)
{

	std::cout << "Socket creado exitosamente" << std::endl;
}

HandleSocket::HandleSocket(const HandleSocket &other) :
	_host(other._host),
	_port(other._port),
	_fd(other._fd)
{
	std::cout << "Other socket creado" << std::endl;
}

HandleSocket &HandleSocket::operator=(const HandleSocket &other)
{
	if (this != &other)
	{
		this->_host = other._host;
		this->_port = other._port;
		this->_fd  = other._fd;
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
