#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <string>
#include <sstream>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

class HandleSocket
{
	private:
		std::string _host;
		int _port;
		int _fd;
		struct sockaddr_storage _addr;
		socklen_t _addrLen;
	public:
		HandleSocket();
		HandleSocket(const std::string &host, const int &port);
		HandleSocket(int fd, const struct sockaddr_storage &addr, socklen_t addrLen);
		HandleSocket(const HandleSocket &other);
		HandleSocket &operator=(const HandleSocket &other);
		~HandleSocket();

	//Functs
		void createSocket();
		void setReuseAddr();
		void bindSocket();
		void listenSocket();
		void setNonBlocking();

	//Getters
		int getFD() const;
		int getPort() const;
		std::string getHost() const;

	//Setters
		void setFD(const int &fd);
		void setPort(const int &port);
		void setHost(const std::string &host);

};
