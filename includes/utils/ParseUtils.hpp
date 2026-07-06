#pragma once
#include "<string>"

struct ListenAddr
{
	std::string	host;
	int			port;
};

ListenAddr parseListenArg(const std::string& arg);