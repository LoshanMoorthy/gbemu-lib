// tile.cc
#include "tile.h"
#include "color.h"
#include "bitwise.h"
#include "mmu.h"

using bitwise::bit_value;

Tile::Tile(Address tile_address, MMU& mmu, uint size_multiplier)
    : size(size_multiplier) {
    // Allocate enough for (8×size) lines × 8 columns
    buffer.resize(TILE_WIDTH_PX * (TILE_HEIGHT_PX * size_multiplier), GBColor::Color0);

    for (uint tile_line = 0; tile_line < TILE_HEIGHT_PX * size_multiplier; tile_line++) {
        // For an 8×8 tile, each line is 2 bytes => offset is tile_line * 2
        // If size_multiplier=2 => we double the line index. Typically 8×16 sprite mode.
        uint index_into_tile = 2 * tile_line;

        Address line_start = tile_address + index_into_tile;

        u8 pixels_1 = mmu.read(line_start);
        u8 pixels_2 = mmu.read(line_start + 1);

        auto pixel_line = get_pixel_line(pixels_1, pixels_2);

        for (uint x = 0; x < TILE_WIDTH_PX; x++) {
            // Each x in 0..7 => map to GBColor
            auto color_idx = pixel_line[x];  // 0..3
            buffer[pixel_index(x, tile_line, size_multiplier)] = get_color(color_idx);
        }
    }
}

GBColor Tile::get_pixel(uint x, uint y) const {
    if (x >= TILE_WIDTH_PX) { return GBColor::Color0; }
    if (y >= (TILE_HEIGHT_PX * size)) { return GBColor::Color0; }
    return buffer[y * TILE_WIDTH_PX + x];
}

auto Tile::get_pixel_line(u8 byte1, u8 byte2) -> std::vector<u8> {
    // Each bit in byte1, byte2 forms a color index
    // For x from 0..7, color = (bit_value(byte2, 7-x) << 1) | bit_value(byte1, 7-x)
    std::vector<u8> pixel_line(8, 0);

    for (u8 i = 0; i < 8; i++) {
        u8 hi = bit_value(byte2, 7 - i);
        u8 lo = bit_value(byte1, 7 - i);
        pixel_line[i] = (u8)((hi << 1) | lo);
    }
    return pixel_line;
}

uint Tile::pixel_index(uint x, uint y, uint size_multiplier) {
    return y * TILE_WIDTH_PX + x;
}
