#pragma once
#include <exception>
#include <string>
#include <sstream>

class ConfigException : public std::exception
{
	private:
		std::string _msg;
		int			_line;

	public:
		ConfigException(const std::string &msg, int line) : _msg(format(msg, line)), _line(line) {}
		~ConfigException() throw() {}
		const char *what() const throw() { return _msg.c_str(); }
		int line() const { return _line; }

	private:
		static std::string format(const std::string &msg, int line)
		{
			std::stringstream ss;
			ss << msg << " (line " << line << ")";
			return ss.str();
		}
};