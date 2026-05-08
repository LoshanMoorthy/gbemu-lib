#pragma once

enum class LogLevel {
	Trace,
	Debug,
	Info,
	Warning,
	Error,
};

void log_set_level(LogLevel level);

void log_trace(const char* fmt, ...);
void log_debug(const char* fmt, ...);
void log_info(const char* fmt, ...);
void log_warn(const char* fmt, ...);
void log_error(const char* fmt, ...);
