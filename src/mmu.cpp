#include "mmu.h"
#include "log.h"
#include "cpu.h"
#include "video.h"
#include "boot.h"
#include "gameboy.h"

MMU::MMU(Cartridge& inCartridge, CPU& inCPU, Video& inVideo, Joypad& inJoypad, Timer& inTimer, Gameboy& inGb)
    : cartridge(inCartridge)
    , cpu(inCPU)
    , video(inVideo)
    , joypad(inJoypad)
    , timer(inTimer)
    , gameboy(inGb) {
    memory.resize(0x10000, 0);
}

u8 MMU::read(const Address& address) const {
    u16 addr = address.value();

    u8 value = 0xFF;

    if (addr == 0xFF04) value = timer.read_div();
    else if (addr == 0xFF05) value = timer.read_tima();
    else if (addr == 0xFF06) value = timer.read_tma();
    else if (addr == 0xFF07) value = timer.read_tac();
    else if (addr < 0x8000)  value = cartridge.read(address);
    else if (addr < 0xA000)  value = memory_read(address);
    else if (addr < 0xC000)  value = memory_read(address);
    else if (addr < 0xE000)  value = memory_read(address);
    else if (addr < 0xFE00)  value = memory_read(Address(addr - 0x2000));
    else if (addr < 0xFEA0)  value = memory_read(address);
    else if (addr < 0xFF00)  value = 0xFF;
    else if (addr < 0xFF80)  value = read_io(address);
    else if (addr < 0xFFFF)  value = memory_read(address);
    else                     value = cpu.interrupt_enabled.value();

    if (on_read_cb) on_read_cb(addr, value);
    return value;
}

void MMU::write(const Address& address, u8 byte) {
    u16 addr = address.value();

    if (on_write_cb) on_write_cb(addr, byte);

    if (addr < 0x8000) {
        cartridge.write(address, byte);
        return;
    }
    else if (addr < 0xA000) {
        memory_write(address, byte);
        return;
    }
    else if (addr < 0xC000) {
        memory_write(address, byte);
        return;
    }
    else if (addr < 0xE000) {
        memory_write(address, byte);
        return;
    }
    else if (addr < 0xFE00) {
        memory_write(Address(addr - 0x2000), byte);
        return;
    }
    else if (addr < 0xFEA0) {
        memory_write(address, byte);
        return;
    }
    else if (addr < 0xFF00) {
        // Unusable region - ignore silently
        return;
    }
    else if (addr < 0xFF80) {
        write_io(address, byte);
        return;
    }
    else if (addr < 0xFFFF) {
        memory_write(address, byte);
        return;
    }
    else {
        // 0xFFFF = Interrupt Enable register
        cpu.interrupt_enabled.set(byte);
        return;
    }
}

bool MMU::boot_rom_active() const {
    return false;
}

u8 MMU::read_io(const Address& address) const {
    u16 addr = address.value();

    // Joypad - return 0xFF
    if (addr == 0xFF00) return joypad.read();

    // Interrupt Flag
    if (addr == 0xFF0F) return cpu.interrupt_flag.value();

    // LCD registers
    if (addr >= 0xFF40 && addr <= 0xFF4B) {
        switch (addr) {
        case 0xFF40: return video.lcd_control.value();
        case 0xFF41: return video.lcd_status.value();
        case 0xFF42: return video.scroll_y.value();
        case 0xFF43: return video.scroll_x.value();
        case 0xFF44: return video.line.value();
        case 0xFF45: return video.ly_compare.value();
        case 0xFF46: return video.dma_transfer.value();
        case 0xFF47: return video.bg_palette.value();
        case 0xFF48: return video.sprite_palette_0.value();
        case 0xFF49: return video.sprite_palette_1.value();
        case 0xFF4A: return video.window_y.value();
        case 0xFF4B: return video.window_x.value();
        }
    }

    return memory_read(address);
}

void MMU::write_io(const Address& address, u8 byte) {
    u16 addr = address.value();

    if (addr == 0xFF04) {
        timer.write_div(byte);
        return;
    }

    if (addr == 0xFF05) {
        timer.write_tima(byte);
        return;
    }

    if (addr == 0xFF06) {
        timer.write_tma(byte);
        return;
    }

    if (addr == 0xFF07) {
        timer.write_tac(byte);
        return;
    }

    if (addr == 0xFF00) {
        joypad.write(byte);
        return;
    }

    // Interrupt Flag
    if (addr == 0xFF0F) {
        cpu.interrupt_flag.set(byte);
        return;
    }

    // LCD registers
    if (addr >= 0xFF40 && addr <= 0xFF4B) {
        switch (addr) {
        case 0xFF40: video.lcd_control.set(byte); break;
        case 0xFF41: video.lcd_status.set(byte); break;
        case 0xFF42: video.scroll_y.set(byte); break;
        case 0xFF43: video.scroll_x.set(byte); break;
        case 0xFF44: video.line.set(0); break;
        case 0xFF45: video.ly_compare.set(byte); break;
        case 0xFF46: video.dma_transfer.set(byte); 
                     {
                        u16 src = (u16)byte << 8;
                        for (int i = 0; i < 160; i++) {
                            u8 val = read(Address(src + i));
                            write(Address(0xFE00 + i), val);
                        }
                     }
                     break;
        case 0xFF47: video.bg_palette.set(byte); break;
        case 0xFF48: video.sprite_palette_0.set(byte); break;
        case 0xFF49: video.sprite_palette_1.set(byte); break;
        case 0xFF4A: video.window_y.set(byte); break;
        case 0xFF4B: video.window_x.set(byte); break;
        }
        return;
    }

    // Serial
    if (addr == 0xFF01) {
        memory_write(address, byte); // store the data byte
        return;
    }

    if (addr == 0xFF02) {
        // bit 7 = transfer start, bit 0 = clock select
        if (byte == 0x81) {
            u8 data = memory_read(Address(0xFF01));
            if (on_serial_cb) on_serial_cb(data);
        }
        memory_write(address, byte);
        return;
    }

    memory_write(address, byte);
}

u8 MMU::memory_read(const Address& address) const {
    return memory[address.value()];
}

void MMU::memory_write(const Address& address, u8 byte) {
    memory[address.value()] = byte;
}