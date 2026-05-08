#pragma once
#include "definitions.h"

class Joypad {
public:
	enum class Button {
		Right, Left, Up, Down,
		A, B, Select, Start
	};

	u8 read() const;
	void write(u8 value);

	void set_button(Button button, bool pressed);

	bool consume_interrupt_request();

private:
	u8 select_bits = 0x30;

	bool right = false;
	bool left = false;
	bool up = false;
	bool down = false;

	bool a = false;
	bool b = false;
	bool select = false;
	bool start = false;

	bool interrupt_requested = false;
};