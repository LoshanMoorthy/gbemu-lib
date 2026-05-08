#include "joypad.h"

u8 Joypad::read() const {
    u8 result = 0xC0 | select_bits | 0x0F;

    // Bit 5 low = action buttons selected
    if ((select_bits & 0x20) == 0) {
        if (a)      result &= ~0x01;
        if (b)      result &= ~0x02;
        if (select) result &= ~0x04;
        if (start)  result &= ~0x08;
    }

    // Bit 4 low = d-pad selected
    if ((select_bits & 0x10) == 0) {
        if (right) result &= ~0x01;
        if (left)  result &= ~0x02;
        if (up)    result &= ~0x04;
        if (down)  result &= ~0x08;
    }

    return result;
}

void Joypad::write(u8 value) {
    select_bits = value & 0x30;
}

void Joypad::set_button(Button button, bool pressed) {
    bool* target = nullptr;

    switch (button) {
    case Button::Right:  target = &right; break;
    case Button::Left:   target = &left; break;
    case Button::Up:     target = &up; break;
    case Button::Down:   target = &down; break;
    case Button::A:      target = &a; break;
    case Button::B:      target = &b; break;
    case Button::Select: target = &select; break;
    case Button::Start:  target = &start; break;
    }

    if (target && !(*target) && pressed) {
        interrupt_requested = true;
    }

    if (target) {
        *target = pressed;
    }
}

bool Joypad::consume_interrupt_request() {
    bool requested = interrupt_requested;
    interrupt_requested = false;
    return requested;
}