#include <cstdarg>
#include <iostream>

#include "log.h"
#include "definitions.h"
#include "string"

static LogLevel current_level = LogLevel::Debug;

void log_set_level(LogLevel level) {
	current_level = level;
}

// Util to convert a log to a numeric rank
static int level_value(LogLevel level) {
	switch (level) {
		case LogLevel::Trace:	return 0;
		case LogLevel::Debug:	return 1;
		case LogLevel::Info:	return 2;
		case LogLevel::Warning: return 3;
		case LogLevel::Error:	return 4;
	}
	return 4;
}

// Helper: printf-style formatting.
static std::string vformat(const char* fmt, va_list args) {
	char buf[1024];
	vsnprintf(buf, sizeof(buf), fmt, args);
	return std::string(buf);
}

// For each log_XX we do a level check, then print:
void log_trace(const char* fmt, ...) {
	if (level_value(current_level) > level_value(LogLevel::Trace)) return;
	va_list args;
	va_start(args, fmt);
	std::string msg = vformat(fmt, args);
	va_end(args);
	std::cout << "[TRACE]" << msg << std::endl;
}

void log_debug(const char* fmt, ...) {
    if (level_value(current_level) > level_value(LogLevel::Debug)) return;
    va_list args;
    va_start(args, fmt);
    std::string msg = vformat(fmt, args);
    va_end(args);
    std::cout << "[DEBUG] " << msg << std::endl;
}

void log_info(const char* fmt, ...) {
    if (level_value(current_level) > level_value(LogLevel::Info)) return;
    va_list args;
    va_start(args, fmt);
    std::string msg = vformat(fmt, args);
    va_end(args);
    std::cout << "[INFO]  " << msg << std::endl;
}

void log_warn(const char* fmt, ...) {
    if (level_value(current_level) > level_value(LogLevel::Warning)) return;
    va_list args;
    va_start(args, fmt);
    std::string msg = vformat(fmt, args);
    va_end(args);
    std::cerr << "[WARN]  " << msg << std::endl;
}

void log_error(const char* fmt, ...) {
    // Always show errors
    va_list args;
    va_start(args, fmt);
    std::string msg = vformat(fmt, args);
    va_end(args);
    std::cerr << "[ERROR] " << msg << std::endl;
}