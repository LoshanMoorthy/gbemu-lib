#include "framebuffer.h"

FrameBuffer::FrameBuffer(uint _width, uint _height)
    : width(_width), height(_height),
    buffer(_width* _height, Color::White) {
}

void FrameBuffer::set_pixel(uint x, uint y, Color color) {
    buffer[pixel_index(x, y)] = color;
}

Color FrameBuffer::get_pixel(uint x, uint y) const {
    return buffer.at(pixel_index(x, y));
}

void FrameBuffer::reset() {
    // Reset all pixels to white (or black, your choice :D)
    for (auto& pixel : buffer) {
        pixel = Color::White;
    }
}

inline uint FrameBuffer::pixel_index(uint x, uint y) const {
    return (y * width) + x;
}
