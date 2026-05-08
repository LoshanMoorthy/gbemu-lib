#include <fstream>

#include "files.h"
#include "definitions.h"
#include "log.h"

std::vector<char> read_bytes(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.good()) {
		fatal_error("Cannot open file: %s", filename.c_str());
	}

	std::streampos size = file.tellg();
	std::vector<char> buffer(static_cast<size_t>(size));

	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), size);
	file.close();
	
	return buffer;
}