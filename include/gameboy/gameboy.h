#pragma once

#include <cstdint>
#include <string>
#include <functional>

namespace gbemu {

// All GameBoy Buttons
enum class Button {
	A, B,
	Start, Select,
	Up, Down, Left, Right
};

// CPU register state - exposed for debuggers/tracers
struct Registers {
	uint8_t a, b, c, d, e, h, l, f;
	uint16_t sp, pc;
};

// Snapshop of the full emulator state - used for save states
struct EmulatorState {
	// filled in when save states are implemented
};

// Main emulator class
//
// Minimal usage:
//   GameBoy gb;
//   gb.load_rom("tetris.gb");
//   gb.on_frame([](const uint32_t* pixels, int w, int h) { ... });
//   gb.on_input([](Button b) -> bool { ... });
//   gb.run();	
class GameBoy {
public:
	GameBoy();
	~GameBoy();

	// Load a .gb ROM file. Returns false if the file couldn't be loaded.
	bool load_rom(const std::string& path);

	// Run at 60fps - blocking, returns when emulator stops
	void run();

	// Step exactly one frame - non-blocking
	void step_frame();

	// -- Required --

	// Raw RGBA pixels, always 160x144
	void on_frame(std::function<void(const uint32_t*, int, int)> callback);

	// Return true if the given button is currently held
	void on_input(std::function<bool(Button)> callback);

	// -- Optional hooks --
	
	// Fires after every CPU instruction - good for debuggers and tracers
	void on_instruction(std::function<void(uint16_t pc, uint8_t opcode, Registers)> callback);

	// Fires on every memory write - good for watchpoints and cheat engines
	void on_memory_write(std::function<void(uint16_t addr, uint8_t value)> callback);

	// Fires on every memory read
	void on_memory_read(std::function<void(uint16_t addr, uint8_t value)> callback);

	// Fires after each scanline (0-143) - good for scanline effects
	void on_scanline(std::function<void(int line, const uint32_t* pixels)> callback);

	// Fires when the GameBoy sends a byte via the link cable
	void on_serial(std::function<void(uint8_t)> callback);

	// -- Save states --
	EmulatorState get_state() const;
	void set_state(const EmulatorState& state);

private:
	struct Impl;
	Impl* impl; // pimpl idiom
};

}