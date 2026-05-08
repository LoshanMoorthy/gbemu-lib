#include "video.h"
#include "color.h"
#include "tile.h"
#include "cpu.h"
#include "cartridge.h"
#include "bitwise.h"
#include "log.h"
#include "gameboy.h"

using bitwise::check_bit;

Video::Video(Gameboy& inGb)
    : gb(inGb)
    , buffer(GAMEBOY_WIDTH, GAMEBOY_HEIGHT) {
    // Initialize registers to 0 if needed
    lcd_control.set(0x91);
    lcd_status.set(0x85);
    scroll_y.set(0x00);
    scroll_x.set(0x00);
    line.set(0x00);
    ly_compare.set(0x00);
    dma_transfer.set(0x00);
    bg_palette.set(0xFC);         
    sprite_palette_0.set(0xFF);
    sprite_palette_1.set(0xFF);
    window_y.set(0x00);
    window_x.set(0x00);
}

// Called after each CPU instruction. 'cycles' = how many cycles that instruction used.
void Video::tick(Cycles cycles) {
    cycle_counter += cycles.cycles;

    switch (current_mode) {
        case VideoMode::ACCESS_OAM:
        if (cycle_counter >= CLOCKS_PER_SCANLINE_OAM) {
            cycle_counter -= CLOCKS_PER_SCANLINE_OAM;
            current_mode = VideoMode::ACCESS_VRAM;
            // Mode 3
            lcd_status.set((lcd_status.value() & 0xFC) | 0x03);
        }
        break;

        case VideoMode::ACCESS_VRAM:
        if (cycle_counter >= CLOCKS_PER_SCANLINE_VRAM) {
            cycle_counter -= CLOCKS_PER_SCANLINE_VRAM;
            current_mode = VideoMode::HBLANK;
            // Mode 0
            lcd_status.set((lcd_status.value() & 0xFC) | 0x00);
        }
        break;

        case VideoMode::HBLANK: {
            if (cycle_counter >= CLOCKS_PER_HBLANK) {
                cycle_counter -= CLOCKS_PER_HBLANK;
                write_scanline(line.value());
                line.increment();

                if (line.value() == 144) {
                    current_mode = VideoMode::VBLANK;
                    // Mode 1
                    lcd_status.set((lcd_status.value() & 0xFC) | 0x01);
                    gb.cpu.interrupt_flag.set_bit_to(0, true);
                }
                else {
                    current_mode = VideoMode::ACCESS_OAM;
                    // Mode 2
                    lcd_status.set((lcd_status.value() & 0xFC) | 0x02);
                }
            }
            break;
        }
        case VideoMode::VBLANK:
        if (line.value() >= 144 && line.value() < 154) {
            if (cycle_counter >= CLOCKS_PER_SCANLINE) {
                cycle_counter -= CLOCKS_PER_SCANLINE;
                line.increment();

                if (line.value() > 153) {
                    line.set(0);
                    current_mode = VideoMode::ACCESS_OAM;
                    // Mode 2
                    lcd_status.set((lcd_status.value() & 0xFC) | 0x02);
                    write_sprites();
                    draw();
                    buffer.reset();
                }
            }
        }
        break;
    }
}

bool Video::display_enabled() const { return check_bit(lcd_control.value(), 7); }
bool Video::window_tile_map() const { return check_bit(lcd_control.value(), 6); }
bool Video::window_enabled() const { return check_bit(lcd_control.value(), 5); }
bool Video::bg_window_tile_data() const { return check_bit(lcd_control.value(), 4); }
bool Video::bg_tile_map_display() const { return check_bit(lcd_control.value(), 3); }
bool Video::sprite_size() const { return check_bit(lcd_control.value(), 2); }
bool Video::sprites_enabled() const { return check_bit(lcd_control.value(), 1); }
bool Video::bg_enabled() const { return check_bit(lcd_control.value(), 0); }

void Video::write_scanline(u8 current_line) {
    if (!display_enabled()) {
        return;
    }

    if (bg_enabled()) {
        draw_bg_line(current_line);
    }
    if (window_enabled()) {
        draw_window_line(current_line);
    }
}

// After all lines are drawn, we can draw all sprites
void Video::write_sprites() {
    if (!sprites_enabled()) return;

    for (uint spriteIndex = 0; spriteIndex < 40; spriteIndex++) {
        draw_sprite(spriteIndex);
    }
}

// Renders background for one line
void Video::draw_bg_line(uint current_line) {
    // Typical addresses
    const Address TILE_SET_ZERO_ADDRESS = 0x8000;
    const Address TILE_SET_ONE_ADDRESS = 0x8800;

    const Address TILE_MAP_ZERO_ADDRESS = 0x9800;
    const Address TILE_MAP_ONE_ADDRESS = 0x9C00;

    // Decide which tile set
    bool use_tile_set_zero = bg_window_tile_data(); // 0x8000
    Address tile_set = use_tile_set_zero ? TILE_SET_ZERO_ADDRESS : TILE_SET_ONE_ADDRESS;

    // Decide which BG tile map
    bool use_tile_map_zero = !bg_tile_map_display();
    Address tile_map = use_tile_map_zero ? TILE_MAP_ZERO_ADDRESS : TILE_MAP_ONE_ADDRESS;

    // For each pixel in [0..159]
    for (uint screen_x = 0; screen_x < GAMEBOY_WIDTH; screen_x++) {
        uint scrolled_x = screen_x + scroll_x.value();
        uint scrolled_y = current_line + scroll_y.value();

        // BG is 256×256, repeated tile map
        uint tile_map_x = scrolled_x % 256;
        uint tile_map_y = scrolled_y % 256;

        // Which tile in the tilemap?
        uint tile_x = tile_map_x / 8;
        uint tile_y = tile_map_y / 8;
        uint tile_index = tile_y * 32 + tile_x;

        // read tile ID
        u8 tile_id = gb.mmu.read(tile_map + tile_index);

        // If using tile_set_one, interpret tile_id as signed
        s16 tile_number = use_tile_set_zero ? tile_id : (s8)tile_id + 128;

        // compute tile address
        Address tile_address = tile_set + (tile_number * 16);

        // pixel inside the tile
        uint pixel_x = tile_map_x % 8;
        uint pixel_y = tile_map_y % 8;

        // read tile
        Tile tile(tile_address, gb.mmu);
        GBColor colorIdx = tile.get_pixel(pixel_x, pixel_y);

        // apply the BG palette
        auto palette = load_palette(bg_palette);
        auto final_color = get_color_from_palette(colorIdx, palette);

        buffer.set_pixel(screen_x, current_line, final_color);
    }
}

static Color get_real_color(u8 pixel_value) {
    // typical “DMG” shading
    switch (pixel_value) {
    case 0: return Color::White;
    case 1: return Color::LightGray;
    case 2: return Color::DarkGray;
    case 3: return Color::Black;
    default:
    fatal_error("Invalid color index for real color: %d", pixel_value);
    return Color::White;
    }
}

// Renders window (similar logic) for one line
void Video::draw_window_line(uint current_line) {
    if (current_line < window_y.value()) {
        return; // Window not yet on screen
    }
    uint win_line = current_line - window_y.value();
    if (win_line >= GAMEBOY_HEIGHT) return;

    // window_x register is offset by 7
    int win_x_offset = (int)window_x.value() - 7;

    const Address TILE_SET_ZERO_ADDRESS = 0x8000;
    const Address TILE_SET_ONE_ADDRESS  = 0x8800;
    const Address TILE_MAP_ZERO_ADDRESS = 0x9800;
    const Address TILE_MAP_ONE_ADDRESS  = 0x9C00;

    bool use_tile_set_zero = bg_window_tile_data();
    Address tile_set = use_tile_set_zero ? TILE_SET_ZERO_ADDRESS : TILE_SET_ONE_ADDRESS;

    // Window uses specific tile map bit (bit 6 of LCD?)
    bool use_window_tile_map_one = window_tile_map();
    Address tile_map = use_window_tile_map_one ? TILE_MAP_ONE_ADDRESS : TILE_MAP_ZERO_ADDRESS;

    for (uint screen_x = 0; screen_x < GAMEBOY_WIDTH; screen_x++) {
        int win_x = (int)screen_x - win_x_offset;
        if (win_x < 0) continue;

        uint tile_x = (uint)win_x / 8;
        uint tile_y = win_line / 8;
        uint tile_index = tile_y * 32 + tile_x;

        u8 tile_id = gb.mmu.read(tile_map + tile_index);
        s16 tile_number = use_tile_set_zero ? tile_id : (s8)tile_id + 128;

        Address tile_address = tile_set + (tile_number * 16);

        uint pixel_x = (uint)win_x % 8;
        uint pixel_y = win_line % 8;

        Tile tile(tile_address, gb.mmu);
        GBColor colorIdx = tile.get_pixel(pixel_x, pixel_y);

        auto palette = load_palette(bg_palette);
        auto final_color = get_color_from_palette(colorIdx, palette);

        buffer.set_pixel(screen_x, current_line, final_color);
    }
}

void Video::draw_sprite(uint sprite_n) {
    // OAM is at 0xFE00
    Address oam_start = Address(0xFE00 + sprite_n * 4);

    u8 posY    = gb.mmu.read(oam_start + 0); // Y pos
    u8 posX    = gb.mmu.read(oam_start + 1); // X pos
    u8 tileNum = gb.mmu.read(oam_start + 2);
    u8 attr    = gb.mmu.read(oam_start + 3);

    // Sprites are offset: posX-8, posY-16
    int sprite_x = (int)posX - 8;
    int sprite_y = (int)posY - 16;

    bool tall_sprites = sprite_size(); // bit 2 LCDC: false=8x8, true=8x16
    uint height = tall_sprites ? 16 : 8;
    if (tall_sprites) tileNum &= 0xFE; // 8x16: ignore bit 0

    bool flip_x = (attr & 0x20) != 0;
    bool flip_y = (attr & 0x40) != 0;
    bool use_pal1 = (attr & 0x10) != 0;
    bool bg_priority = (attr & 0x80) != 0;

    const ByteRegister& pal_reg = use_pal1 ? sprite_palette_1 : sprite_palette_0;
    auto palette = load_palette(pal_reg);

    Address tile_address = Address(0x8000 + tileNum * 16);
    Tile tile(tile_address, gb.mmu, tall_sprites ? 2 : 1);

    for (uint ty = 0; ty < height; ty++) {
        int screen_y = sprite_y + (int)ty;
        if (screen_y < 0 || screen_y >= (int)GAMEBOY_HEIGHT) continue;

        uint flipped_y = flip_y ? (height - 1 - ty) : ty;

        for (uint tx = 0; tx < 8; tx++) {
            int screen_x = sprite_x + (int)tx;
            if (screen_x < 0 || screen_x >= (int)GAMEBOY_WIDTH) continue;

            uint flipped_x = flip_x ? (7 - tx) : tx;

            GBColor colorIdx = tile.get_pixel(flipped_x, flipped_y);
            if (colorIdx == GBColor::Color0) continue; // transparent

            // bg_priority: sprite is behind BG colors 1-3
            if (bg_priority) {
                Color existing = buffer.get_pixel(screen_x, screen_y);
                if (existing != Color::White) continue;
            }

            auto final_color = get_color_from_palette(colorIdx, palette);
            buffer.set_pixel(screen_x, screen_y, final_color);
        }
    }
}

Palette Video::load_palette(const ByteRegister& palette_reg) {
    using bitwise::compose_bits;
    auto p = palette_reg.value();

    // Each pair of bits is a color
    // bits 0-1 => color0, bits 2-3 => color1, bits 4-5 => color2, bits 6-7 => color3
    u8 c0 = p & 0x03;         
    u8 c1 = (p >> 2) & 0x03;  
    u8 c2 = (p >> 4) & 0x03;
    u8 c3 = (p >> 6) & 0x03;

    auto col0 = get_real_color(c0);
    auto col1 = get_real_color(c1);
    auto col2 = get_real_color(c2);
    auto col3 = get_real_color(c3);

    return { col0, col1, col2, col3 };
}

Color Video::get_color_from_palette(GBColor color, const Palette& pal) {
    switch (color) {
    case GBColor::Color0: return pal.color0;
    case GBColor::Color1: return pal.color1;
    case GBColor::Color2: return pal.color2;
    case GBColor::Color3: return pal.color3;
    }
}

void Video::register_vblank_callback(const vblank_callback_t& cb) {
    vblank_callback = cb;
}

// Called at end of VBlank (or start?), to deliver the final buffer
void Video::draw() {
    if (vblank_callback) {
        vblank_callback(buffer);
    }
}