#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

class HandleSocket
{
	private:
		int _fd;
		std::string _host;
		int _port;
	public:
		HandleSocket();
		HandleSocket(const std::string &host, const int &port);
		HandleSocket(const HandleSocket &other);
		HandleSocket &operator=(const HandleSocket &other);
	//Functs

	//Getters
		int getFD() const;
		int getPort() const;
		std::string getHost() const;
	//Setters
		void setFD(const int &fd);
		void setPort(const int &port);
		void setHost(const std::string &host);

};
