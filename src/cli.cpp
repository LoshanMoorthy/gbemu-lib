#include <iostream>

#include "cli.h"

Options get_options(int argc, char* argv[]) {
	Options opts;
	if (argc < 2) {
		std::cerr << "Please provide a ROM file.\n";
		exit(1);
	}
	opts.filename = argv[1];

	for (int i = 2; i < argc; i++) {
		std::string arg = argv[i];
		if (arg == "--debug") opts.deubgger = true;
		else if (arg == "--trace") opts.trace = true;
		else if (arg == "--silent") opts.disable_logs = true;
		else if (arg == "--exit-on-infinite-jr") opts.exit_on_infinite_jr = true;
		else {
			std::cerr << "Unknown option: " << arg << "\n";
		}
	}
	return opts;
}