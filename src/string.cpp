#include <cstdio>
#include <sstream>

#include "gb_string.h"

std::string str_format(const char* fmt, va_list args) {
	char buf[8192];
	vsnprintf(buf, sizeof(buf), fmt, args);
	return std::string(buf);
}

std::string str_format(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	std::string result = str_format(fmt, args);
	va_end(args);
	return result;
}

std::vector<std::string> split(std::string str, char delim) {
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delim)) {
		result.push_back(item);
	}
	return result;
}
	

