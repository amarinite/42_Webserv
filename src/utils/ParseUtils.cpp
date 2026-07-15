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

bool isHttpCodeValid(int code)
{
	static const int validCodes[] = {
		100, 101,
		200, 201, 202, 204,
		301, 302, 303, 307, 308,
		400, 401, 403, 404, 405, 408, 409, 413,
		500, 501, 502, 503, 504
	};
	static const size_t validCodesCount =
		sizeof(validCodes) / sizeof(validCodes[0]);

	for (size_t i = 0; i < validCodesCount; i++)
	{
		if (validCodes[i] == code)
			return true;
	}
	
	std::stringstream ss;
	ss << code;
	throw std::invalid_argument("invalid HTTP status code: " + ss.str());
}

static std::string normalize(const std::string& raw)
{
	std::string clean = raw;

	if (clean.size() > 1 && clean[clean.size() - 1] == '/')
		clean.erase(clean.size() - 1);

	return clean;
}

std::string	parseFsPath(const std::string& arg)
{
	if (arg[0] != '/' && arg[0] != '.' && arg[0] != '~')
		throw std::invalid_argument("path must start with '/', '.', or '~': " + arg);

	return normalize(arg);
}