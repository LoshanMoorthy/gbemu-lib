#include "gameboy/gameboy.h"
#include "gameboy.h"
#include "cartridge.h"
#include "files.h"

#include <fstream>
#include <stdexcept>

namespace gbemu {

struct GameBoy::Impl {
    // Internal emulator
    std::unique_ptr<Gameboy> core;

    // Config
    Config config;

    // State
    bool running = false;
    bool paused  = false;
    bool loaded  = false;

	// Callbacks
    std::function<void(const uint32_t*, int, int)>      frame_cb;
    std::function<bool(Button)>                         input_cb;
    std::function<void(uint16_t, uint8_t, Registers)>   instruction_cb;
    std::function<void(uint16_t, uint8_t)>              memory_write_cb;
    std::function<void(uint16_t, uint8_t)>              memory_read_cb;
    std::function<void(int, const uint32_t*)>           scanline_cb;
    std::function<void(uint8_t)>                        serial_cb;
    std::function<void()>                               tick_cb;
    std::function<void(LogLevel, const std::string&)>   log_cb;

    // Helper
    void log(LogLevel level, const std::string& msg) {
        if (log_cb) log_cb(level, msg);
    }
};

// -- GameBoy --

GameBoy::GameBoy()  : impl(new Impl()) {}
GameBoy::~GameBoy() { delete impl; }

Error GameBoy::load_rom(const std::string& path) {
    std::ifstream f(path);
    if (!f.good()) {
        impl->log(LogLevel::Error, "File not found: " + path);
        return Error::FileNotFound;
    }
    f.close();

    try {
        auto raw = read_bytes(path);
        std::vector<u8> rom_data(raw.begin(), raw.end());

        Options options;
        options.disable_logs = !impl->config.enable_logging;

        impl->core = std::make_unique<Gameboy>(rom_data, options);
        impl->loaded = true;
        impl->log(LogLevel::Info, "ROM loaded: " + path);
        return Error::None;
    }
    catch (const std::exception& e) {
        impl->log(LogLevel::Error, std::string("Failed to load ROM: ") + e.what());
        return Error::InvalidROM;
    }
    catch (...) {
        impl->log(LogLevel::Error, "Unknown exception during load");
        return Error::InvalidROM;
    }
}

void GameBoy::configure(const Config& config) {
    impl->config = config;
}

void GameBoy::run() {
    if (!impl->loaded) {
        impl->log(LogLevel::Error, "No ROM loaded");
        return;
    }
    if (impl->running) {
        impl->log(LogLevel::Warning, "Already running");
        return;
    }

    impl->running = true;
    impl->paused  = false;
    impl->log(LogLevel::Info, "Emulator started");

    impl->core->run(
        [this]() -> bool {
            if (impl->tick_cb) impl->tick_cb();

            // Update joypad state from input callback
            if (impl->input_cb) {
                impl->core->joypad.set_button(Joypad::Button::A, impl->input_cb(Button::A));
                impl->core->joypad.set_button(Joypad::Button::B, impl->input_cb(Button::B));
                impl->core->joypad.set_button(Joypad::Button::Start, impl->input_cb(Button::Start));
                impl->core->joypad.set_button(Joypad::Button::Select, impl->input_cb(Button::Select));
                impl->core->joypad.set_button(Joypad::Button::Up, impl->input_cb(Button::Up));
                impl->core->joypad.set_button(Joypad::Button::Down, impl->input_cb(Button::Down));
                impl->core->joypad.set_button(Joypad::Button::Left, impl->input_cb(Button::Left));
                impl->core->joypad.set_button(Joypad::Button::Right, impl->input_cb(Button::Right));
            }

            return !impl->running;
        },
        [this](const FrameBuffer& fb) {
            if (impl->paused) return;
            if (!impl->frame_cb) return;

            static std::vector<uint32_t> pixels(160 * 144);
            for (uint y = 0; y < 144; y++) {
                for (uint x = 0; x < 160; x++) {
                    Color c = fb.get_pixel(x, y);
                    uint32_t rgba;
                    switch (c) {
                        case Color::White:     rgba = 0xFF9BBC0F; break;
                        case Color::LightGray: rgba = 0xFF8BAC0F; break;
                        case Color::DarkGray:  rgba = 0xFF306230; break;
                        case Color::Black:     rgba = 0xFF0F380F; break;
                        default:               rgba = 0xFF9BBC0F; break;
                    }
                    pixels[y * 160 + x] = rgba;
                }
            }
            impl->frame_cb(pixels.data(), 160, 144);
        }
    );

    impl->running = false;
    impl->log(LogLevel::Info, "Emulator stopped");
}

void GameBoy::step_frame() {
    if (!impl->loaded || !impl->running || impl->paused) return;
    // TODO: expose single frame step from core
}

// -- Lifecycle --

void GameBoy::pause() {
    if (!impl->running || impl->paused) return;
    impl->paused = true;
    impl->log(LogLevel::Info, "Emulator paused");
}

void GameBoy::resume() {
    if (!impl->running || !impl->paused) return;
    impl->paused = false;
    impl->log(LogLevel::Info, "Emulator resumed");
}

void GameBoy::stop() {
    impl->running = false;
    impl->log(LogLevel::Info, "Emulator stopped");
}

bool GameBoy::is_running() const { return impl->running; }
bool GameBoy::is_paused()  const { return impl->paused; }
bool GameBoy::is_loaded()  const { return impl->loaded; }

// -- Callbacks --

void GameBoy::on_frame(std::function<void(const uint32_t*, int, int)> cb) { impl->frame_cb = cb; }
void GameBoy::on_input(std::function<bool(Button)> cb) { impl->input_cb = cb; }
void GameBoy::on_instruction(std::function<void(uint16_t, uint8_t, Registers)> cb) { impl->instruction_cb = cb; }
void GameBoy::on_memory_write(std::function<void(uint16_t, uint8_t)> cb) { impl->memory_write_cb = cb; }
void GameBoy::on_memory_read(std::function<void(uint16_t, uint8_t)> cb) { impl->memory_read_cb = cb; }
void GameBoy::on_scanline(std::function<void(int, const uint32_t*)> cb) { impl->scanline_cb = cb; }
void GameBoy::on_serial(std::function<void(uint8_t)> cb) { impl->serial_cb = cb; }
void GameBoy::on_tick(std::function<void()> cb) { impl->tick_cb = cb; }
void GameBoy::on_log(std::function<void(LogLevel, const std::string&)> cb) { impl->log_cb = cb; }

// -- Save states --

EmulatorState GameBoy::get_state() const { return EmulatorState{}; }
void GameBoy::set_state(const EmulatorState&) {}

}