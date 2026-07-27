#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

#include <HttpException.hpp>

std::string readFile(const std::string& path);
std::string findFileExtension(const std::string &path);
bool        validatePathDir(const std::string &fullpath);
bool        validateFile(const std::string &path);
bool        validateDir(const std::string &dir);
void        removeFile(const std::string &path);