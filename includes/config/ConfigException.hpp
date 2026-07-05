#pragma once
#include <exception>
#include <string>

class ConfigException : public std::exception
{
	private:
		std::string	_msg;
		int			_line;
	public:
		ConfigException(const std::string &msg, int line) : _msg(msg), _line(line) {}
		~ConfigException() throw() {}
		const char *what() const throw() { return _msg.c_str(); }
		int line() const { return _line; }
};