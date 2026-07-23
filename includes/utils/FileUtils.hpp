#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <HttpException.hpp>

std::string readFile(const std::string& path);
std::string validatePathDir(const std::string &fullpath);
std::string findFileExtension(std::string &path);
bool        validateFile(const std::string &path);
bool        validateDir(const std::string &dir);
void        removeFile(std::string &path);