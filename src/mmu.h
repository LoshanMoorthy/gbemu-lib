#pragma once

#include <vector>

#include "definitions.h"
#include "cartridge.h"
#include "address.h"
#include "joypad.h"
#include "timer.h"

class CPU;
class Gameboy;
class Video;

/*
	The MMU handles reading/writing the entire
	64KB memory space. Part of it is ROM, part is RAM,
	and part is I/O.
*/

class MMU {
public:
	MMU(Cartridge& inCartridge, CPU& inCPU, Video& inVideo, Joypad& inJoypad, Timer& inTimer, Gameboy& inGb);

	u8 read(const class Address& address) const;
	void write(const class Address& address, u8 byte);

private:
	// If there's a boot ROM, we might check wether its still active?
	bool boot_rom_active() const;

	// r/w registers
	u8 read_io(const class Address& address) const;
	void write_io(const class Address& address, u8 byte);

	// r/w typical RAM areas from our mem vec.
	u8 memory_read(const class Address& address) const;
	void memory_write(const class Address& address, u8 byte);

private:
	Cartridge& cartridge;
	CPU& cpu;
	Video& video;
	Joypad& joypad;
	Timer& timer;
	Gameboy& gameboy;

	std::vector<u8> memory;
};