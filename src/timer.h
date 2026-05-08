#pragma once

#include "definitions.h"

class Timer {
public:
    void tick(uint32_t cycles);

    u8 read_div() const;
    u8 read_tima() const;
    u8 read_tma() const;
    u8 read_tac() const;

    void write_div(u8 value);
    void write_tima(u8 value);
    void write_tma(u8 value);
    void write_tac(u8 value);

    bool consume_interrupt_request();

private:
    uint32_t div_counter = 0;
    uint32_t timer_counter = 0;

    u8 div = 0xAB;   // post-boot
    u8 tima = 0x00;
    u8 tma = 0x00;
    u8 tac = 0xF8;

    bool interrupt_requested = false;

    bool timer_enabled() const;
    uint32_t timer_frequency_cycles() const;
};