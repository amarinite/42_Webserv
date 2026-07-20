#include "FileUtils.hpp"

std::string readFile(const std::string& path)
{
	std::ifstream file(path.c_str());
	if (!file.is_open())
		throw std::runtime_error("could not open config file: " + path);

	std::stringstream buffer;
	buffer << file.rdbuf();

	if (file.bad())
		throw std::runtime_error("error reading config file: " + path);

	return buffer.str();
}