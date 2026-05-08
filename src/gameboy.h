#pragma once

#include "definitions.h"
#include "cli.h"
#include "log.h"
#include "cartridge.h"  
#include "cpu.h"         
#include "video.h"       
#include "mmu.h"  
#include "joypad.h"
#include "timer.h"

#include <memory>
#include <functional>
#include <vector>

class FrameBuffer;

using vblank_callback_t = std::function<void(const FrameBuffer&)>;
using should_close_callback_t = std::function<bool()>;

class Gameboy {
private:
    std::shared_ptr<Cartridge> cartridge;

public:
    CPU cpu;
    Video video;
    Joypad joypad;
    Timer timer;
    MMU mmu;

    Gameboy(const std::vector<u8>& cartridge_data, 
            Options& options,
            const std::vector<u8>& save_data = {});

    void run(
        const should_close_callback_t& _should_close_callback,
        const vblank_callback_t& _vblank_callback
    );

    auto get_cartridge_ram() const -> const std::vector<u8>&;

private:
    void tick();
    uint elapsed_cycles = 0;
    should_close_callback_t should_close_callback;
};
