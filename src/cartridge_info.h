#pragma once

#include <string>
#include <memory>
#include <vector>

#include "definitions.h"

const int TITLE_LENGTH = 11;

namespace header {
	const int entry_point		= 0x100;
	const int logo				= 0x104;
	const int title				= 0x134;
	const int manufacturer_code = 0x13F;
	const int cgb_flag			= 0x143;
	const int new_licence_code  = 0x144;
	const int sgb_flag			= 0x146;
	const int cartridge_type	= 0x147;
	const int rom_size			= 0x148;
	const int ram_size			= 0x149;
	const int destination_code  = 0x14A;
	const int version_number	= 0x14C;
	const int header_checksum   = 0x14D;
	const int global_checksum	= 0x14E;
}

// Type
enum class CartridgeType {
	ROMOnly,
	MBC1,
	MBC2,
	MBC3,
	MBC4,
	MBC5,
	Unknown,
};

// Helper for reading the type code
CartridgeType get_type(u8 type);
std::string describe(CartridgeType type);

std::string get_title(std::vector<u8>& rom);

std::string get_license(u16 old_license, u16 new_license);

// ROM sizes
enum class ROMSize {
	KB32,
	KB64,
	KB128,
	KB256,
	KB512,
	MB1,
	MB2,
	MB4,
	MB1p1,
	MB1p2,
	MB1p5,
};
ROMSize get_rom_size(u8 size_code);
std::string describe(ROMSize size);

// RAM sizes
enum RAMSize {
	None,
	KB2,
	KB8,
	KB32,
	KB128,
	KB64,
};
RAMSize get_ram_size(u8 size_code);
uint get_actual_ram_size(RAMSize size_code);
std::string describe(RAMSize size);

enum class Destination {
	Japanese,
	NonJapanese,
};
Destination get_destination(u8 destination);
std::string describe(Destination destination);

class CartridgeInfo {
public:
	std::string title;
	CartridgeType type;
	Destination destination;
	ROMSize rom_size;
	RAMSize ram_size;
	std::string license_code;
	u8 version;

	u16 header_checksum;
	u16 global_checksum;

	bool supports_cgb;
	bool supports_sgb;
};

std::unique_ptr<CartridgeInfo> get_info(std::vector<u8> rom);