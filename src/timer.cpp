#include "timer.h"

void Timer::tick(uint32_t cycles) {
    div_counter += cycles;

    while (div_counter >= 256) {
        div_counter -= 256;
        div++;
    }

    if (!timer_enabled()) {
        return;
    }

    timer_counter += cycles;

    uint32_t threshold = timer_frequency_cycles();

    while (timer_counter >= threshold) {
        timer_counter -= threshold;

        if (tima == 0xFF) {
            tima = tma;
            interrupt_requested = true;
        }
        else {
            tima++;
        }
    }
}

u8 Timer::read_div() const {
    return div;
}

u8 Timer::read_tima() const {
    return tima;
}

u8 Timer::read_tma() const {
    return tma;
}

u8 Timer::read_tac() const {
    return tac | 0xF8;
}

void Timer::write_div(u8 value) {
    unused(value);
    div = 0;
    div_counter = 0;
}

void Timer::write_tima(u8 value) {
    tima = value;
}

void Timer::write_tma(u8 value) {
    tma = value;
}

void Timer::write_tac(u8 value) {
    tac = (value & 0x07) | 0xF8;
}

bool Timer::consume_interrupt_request() {
    bool requested = interrupt_requested;
    interrupt_requested = false;
    return requested;
}

bool Timer::timer_enabled() const {
    return (tac & 0x04) != 0;
}

uint32_t Timer::timer_frequency_cycles() const {
    switch (tac & 0x03) {
    case 0x00: return 1024; // 4096 Hz
    case 0x01: return 16;   // 262144 Hz
    case 0x02: return 64;   // 65536 Hz
    case 0x03: return 256;  // 16384 Hz
    }

    return 1024;
}