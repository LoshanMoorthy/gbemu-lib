#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <string>

// Convenience:
using uint = unsigned int;
using u8   = uint8_t;
using u16  = uint16_t;
using s8   = int8_t;
using s16  = int16_t;

// Helper to mark variables unused??
template <typename... T>
inline void unused(T&&...) {}

// Errors that abort
#define fatal_error(fmt, ...) do {      \
    log_error("Fatal error in %s:%d", __func__, __LINE__);  \
    log_error(fmt, ##__VA_ARGS__);      \
    std::exit(1);                       \
} while(0)

// Forward decleration
void log_error(const char* fmt, ...);

struct Cycles {
    explicit Cycles(uint32_t c) : cycles(c) {}
    uint32_t cycles;
};

struct Noncopyable {
    auto operator=(const Noncopyable&) -> Noncopyable& = delete;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable() = default;
    ~Noncopyable() = default;
};

enum class GBColor {
    Color0,
    Color1,
    Color2,
    Color3
};

enum class Color {
    White,
    LightGray,
    DarkGray,
    Black
};