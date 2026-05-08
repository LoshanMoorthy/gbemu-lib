#pragma once

#include "address.h"
#include "definitions.h"
class MMU;

#include <array>
#include <vector>

static const uint TILE_HEIGHT_PX = 8;
static const uint TILE_WIDTH_PX = 8;
static const uint TILE_BYTES = 16;

class Tile {
public:
    Tile(Address tile_address, MMU& mmu, uint size_multiplier = 1);

    auto get_pixel(uint x, uint y) const->GBColor;

private:
    static auto pixel_index(uint x, uint y, uint size_multiplier) -> uint;
    static auto get_pixel_line(u8 byte1, u8 byte2) -> std::vector<u8>;

    // We store color indexes in buffer, up to 2× the height if size_multiplier=2
    std::vector<GBColor> buffer;
    uint size;
};
