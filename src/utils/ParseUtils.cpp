#include "ParseUtils.hpp"

int parseNumber(const std::string& str)
{
	if (str.empty()) 
		throw std::invalid_argument("Empty string passed to parseNumber");

	std::stringstream ss(str);
	int value;

	if ((ss >> value) && (ss.eof())) 
		return value;
	
	throw std::invalid_argument("Invalid integer format: \"" + str + "\"");
}

ListenAddr parseListenArg(const std::string& arg)
{
	size_t colon = arg.find(':');
	std::string hostPart;
	std::string portPart;

	if (colon == std::string::npos)
	{
		bool allDigits = !arg.empty() &&
			arg.find_first_not_of("0123456789") == std::string::npos;
		if (allDigits)
		{
			hostPart = "";
			portPart = arg;
		}
		else
		{
			hostPart = arg;
			portPart = "";
		}
	}
	else
	{
		hostPart = arg.substr(0, colon);
		portPart = arg.substr(colon + 1);
	}

	ListenAddr result;
	result.host = hostPart.empty() ? "0.0.0.0" : hostPart;
	if (portPart.empty())
		result.port = 80;
	else
	{
		int port = parseNumber(portPart);
		if (port < 1 || port > 65535)
			throw std::invalid_argument("listen: port out of range: " + portPart);
		result.port = port;
	}

	return result;
}