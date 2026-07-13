#pragma once
#include <string>
#include <sstream>
#include <stdexcept>

struct ListenAddr
{
	std::string	host;
	int			port;
};

inline bool operator==(const ListenAddr& a, const ListenAddr& b)
{
	return a.host == b.host && a.port == b.port;
}

int			parseNumber(const std::string& str);
ListenAddr	parseListenArg(const std::string& arg);