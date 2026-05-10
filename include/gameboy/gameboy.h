#pragma once

#include <cstdint>
#include <string>
#include <functional>

namespace gbemu {

// Library version
constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 1;
constexpr int VERSION_PATCH = 0;

// Error codes returned by the API
enum class Error {
	None,
	FileNotFound,
	InvalidROM,
	UnsupportedCartridge,
	AlreadyRunning,
	NotLoaded
};

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

// Configuration passed before starting the emulator
struct Config {
	bool skip_boot_rom		= true;
	int  speed_multiplier	= 1; // 1 = normal, 2 = double speed etc.
	bool enable_logging		= true;
};

enum class LogLevel {
	Info,
	Warning,
	Error
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

	// -------------------------------------------------------------------------
	// Core
	// -------------------------------------------------------------------------

	// Load a .gb ROM file. Returns false if the file couldn't be loaded.
	Error load_rom(const std::string& path);

	// Apply configuration - must be called before run()
	void configure(const Config& config);

	// Run at 60fps - blocking, returns when emulator stops
	void run();

	// Step exactly one frame - non-blocking
	void step_frame();

	// -------------------------------------------------------------------------
	// Lifecycle
	// -------------------------------------------------------------------------

	void pause();
	void resume();
	void stop();

	bool is_running() const;
	bool is_paused()  const;
	bool is_loaded()  const;

	// -------------------------------------------------------------------------
	// Required callbacks
	// -------------------------------------------------------------------------

	// Raw RGBA pixels, always 160x144
	void on_frame(std::function<void(const uint32_t*, int, int)> callback);

	// Return true if the given button is currently held
	void on_input(std::function<bool(Button)> callback);

	// -------------------------------------------------------------------------
	// Optional hooks - all default to no-op if not set
	// -------------------------------------------------------------------------
	
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

	// Called once per frame - use this to pump platform events
	void on_tick(std::function<void()> callback);

	// Fires when the emulator has a log message
	void on_log(std::function<void(LogLevel, const std::string&)> callback);

	// -------------------------------------------------------------------------
	// Save states
	// -------------------------------------------------------------------------

	EmulatorState get_state() const;
	void set_state(const EmulatorState& state);

	// -------------------------------------------------------------------------
	// Versioning
	// -------------------------------------------------------------------------

	static int version_major() { return VERSION_MAJOR; }
	static int version_minor() { return VERSION_MINOR; }
	static int version_patch() { return VERSION_PATCH; }

private:
	struct Impl;
	Impl* impl; // pimpl idiom
};

}