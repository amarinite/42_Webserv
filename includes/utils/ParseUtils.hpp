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

inline bool operator<(const ListenAddr& a, const ListenAddr& b)
{
	if (a.host != b.host)
		return a.host < b.host;
	return a.port < b.port;
}

int			parseNumber(const std::string& str);
ListenAddr	parseListenArg(const std::string& arg);
bool		isHttpCodeValid(int code);
std::string	parseFsPath(const std::string& arg);

template <typename T>
std::string numberToString(T value)
{
	std::stringstream ss;
	ss << value;
	if (ss.fail())
		throw std::invalid_argument("Failed to convert number to string");
	return ss.str();
}