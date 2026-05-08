#include "gameboy.h"
#include "cpu.h"
#include "mmu.h"
#include "video.h"
#include "cartridge.h"
#include "files.h"

#include <chrono>
#include <thread>

Gameboy::Gameboy(const std::vector<u8>& cartridge_data,
                 Options& options,
                 const std::vector<u8>& save_data)
    : cartridge(get_cartridge(cartridge_data, save_data))
    , cpu(*this, nullptr, options)       
    , video(*this)  
    , joypad()
    , timer()
    , mmu(*cartridge, cpu, video, joypad, timer, *this) 
{
    cpu.setMMUPointer(&mmu);

    if (options.disable_logs)
        log_set_level(LogLevel::Error);
    else if (options.trace)
        log_set_level(LogLevel::Trace);
    else
        log_set_level(LogLevel::Info);
}

void Gameboy::run(
    const should_close_callback_t& _should_close_callback,
    const vblank_callback_t& _vblank_callback
) {
    should_close_callback = _should_close_callback;
    video.register_vblank_callback(_vblank_callback);

    const int CYCLES_PER_FRAME = 70224;
    auto frame_start = std::chrono::steady_clock::now();

    while (true) {
        if (should_close_callback()) break;

        int cycles_this_frame = 0;
        while (cycles_this_frame < CYCLES_PER_FRAME) {
            auto c = cpu.tick();
            timer.tick(c.cycles);

            if (timer.consume_interrupt_request())
                cpu.interrupt_flag.set_bit_to(2, true);
            if (joypad.consume_interrupt_request())
                cpu.interrupt_flag.set_bit_to(4, true);

            video.tick(c);
            cycles_this_frame += c.cycles;
        }

        auto frame_end = std::chrono::steady_clock::now();
        auto frame_duration = std::chrono::duration_cast<std::chrono::microseconds>(
            frame_end - frame_start).count();

        if (frame_duration < 16742) {
            std::this_thread::sleep_for(
                std::chrono::microseconds(16742 - frame_duration));
        }

        frame_start = std::chrono::steady_clock::now();
    }
}

void Gameboy::tick() {
    auto cycles = cpu.tick();

    timer.tick(cycles.cycles);

    if (timer.consume_interrupt_request()) {
        cpu.interrupt_flag.set_bit_to(2, true);
    }

    if (joypad.consume_interrupt_request()) {
        cpu.interrupt_flag.set_bit_to(4, true);
    }

    elapsed_cycles += cycles.cycles;
    video.tick(cycles);
}

auto Gameboy::get_cartridge_ram() const -> const std::vector<u8>& {
    return cartridge->get_cartridge_ram();
}