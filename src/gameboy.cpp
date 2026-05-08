#include "gameboy/gameboy.h"
#include "gameboy.h"
#include "cartridge.h";
#include "files.h"

namespace gbemu {

struct GameBoy::Impl {
    // Internal emulator
    std::unique_ptr<Gameboy> core; 
     
	// Callbacks
    std::function<void(const uint32_t*, int, int)>    frame_cb;
    std::function<bool(Button)>                       input_cb;
    std::function<void(uint16_t, uint8_t, Registers)> instruction_cb;
    std::function<void(uint16_t, uint8_t)>            memory_write_cb;
    std::function<void(uint16_t, uint8_t)>            memory_read_cb;
    std::function<void(int, const uint32_t*)>         scanline_cb;
    std::function<void(uint8_t)>                      serial_cb;

    bool running = false;
};

// -- GameBoy --

GameBoy::GameBoy()  : impl(new Impl()) {}
GameBoy::~GameBoy() { delete impl; }

bool GameBoy::load_rom(const std::string& path) {
    // TODO: wire up the cartridge loader
    return false;
}

void GameBoy::run() {
    impl->running = true;
    while (impl->running) {
        step_frame();
    }
}

void GameBoy::step_frame() {
    // TODO: wire up frame loop
}

// -- Callbacks --
void GameBoy::on_frame(std::function<void(const uint32_t*, int, int)> cb) { impl->frame_cb = cb; }
void GameBoy::on_input(std::function<bool(Button)> cb) { impl->input_cb = cb; }
void GameBoy::on_instruction(std::function<void(uint16_t, uint8_t, Registers)> cb) { impl->instruction_cb = cb; }
void GameBoy::on_memory_write(std::function<void(uint16_t, uint8_t)> cb) { impl->memory_write_cb = cb; }
void GameBoy::on_memory_read(std::function<void(uint16_t, uint8_t)> cb) { impl->memory_read_cb = cb; }
void GameBoy::on_scanline(std::function<void(int, const uint32_t*)> cb) { impl->scanline_cb = cb; }
void GameBoy::on_serial(std::function<void(uint8_t)> cb) { impl->serial_cb = cb; }

// -- Save states --

EmulatorState GameBoy::get_state() const { return EmulatorState{}; }
void GameBoy::set_state(const EmulatorState&) {}

}