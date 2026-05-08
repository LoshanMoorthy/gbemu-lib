#pragma once

#include <functional>
#include <vector>

#include "definitions.h"
#include "framebuffer.h"
#include "tile.h"
#include "register.h"
#include "mmu.h"

class Gameboy; // for now

using vblank_callback_t = std::function<void(const FrameBuffer&)>;

enum class VideoMode {
    ACCESS_OAM,
    ACCESS_VRAM,
    HBLANK,
    VBLANK,
};

struct Palette {
    Color color0;
    Color color1;
    Color color2;
    Color color3;
};

class Video {
public:
    Video(Gameboy& inGb);

    // Called each time we run an instruction, passing how many cycles it used
    void tick(Cycles cycles);

    // A callback so your main program can fetch the final frame
    void register_vblank_callback(const vblank_callback_t& cb);

    // For the 0xFF40..0xFF4B registers:
    ByteRegister lcd_control;   // 0xFF40
    ByteRegister lcd_status;    // 0xFF41
    ByteRegister scroll_y;      // 0xFF42
    ByteRegister scroll_x;      // 0xFF43
    ByteRegister line;          // 0xFF44
    ByteRegister ly_compare;    // 0xFF45
    ByteRegister dma_transfer;  // 0xFF46
    ByteRegister bg_palette;    // 0xFF47
    ByteRegister sprite_palette_0; // 0xFF48
    ByteRegister sprite_palette_1; // 0xFF49
    ByteRegister window_y;      // 0xFF4A
    ByteRegister window_x;      // 0xFF4B

    // Accessor for the final rendered FrameBuffer
    const FrameBuffer& get_framebuffer() const { return buffer; }

private:
    void draw();
    void write_scanline(u8 current_line);
    void write_sprites();

    void draw_bg_line(uint current_line);
    void draw_window_line(uint current_line);
    void draw_sprite(uint sprite_n);

    // Utility
    bool display_enabled()   const;
    bool window_enabled()    const;
    bool bg_enabled()        const;
    bool sprites_enabled()   const;
    bool sprite_size()       const;
    bool bg_window_tile_data()  const;
    bool bg_tile_map_display()  const;
    bool window_tile_map()      const;

    Palette load_palette(const ByteRegister& palette_reg);
    Color   get_color_from_palette(GBColor color, const Palette& pal);

    // Some constants
    static const uint GAMEBOY_WIDTH = 160;
    static const uint GAMEBOY_HEIGHT = 144;
    static const uint CLOCKS_PER_SCANLINE_OAM = 80;
    static const uint CLOCKS_PER_SCANLINE_VRAM = 172;
    static const uint CLOCKS_PER_HBLANK = 204;
    static const uint CLOCKS_PER_SCANLINE = 456; // sum of above
    static const uint VBLANK_LINES = 10;  // lines 144..153

    // Data
    Gameboy& gb;

    FrameBuffer buffer; // 160×144 final
    VideoMode current_mode = VideoMode::ACCESS_OAM;
    uint cycle_counter = 0;

    vblank_callback_t vblank_callback;
};