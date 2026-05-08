#pragma once

#include <string>

struct Options {
	bool deubgger = false;
	bool trace = false;
	bool disable_logs = false;
	bool exit_on_infinite_jr = false;
	std::string filename;
};

Options get_options(int argc, char* argv[]);