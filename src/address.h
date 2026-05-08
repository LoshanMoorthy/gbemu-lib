#pragma once

#include "definitions.h"
#include "register.h"

/*

	A small wrapper for a 16-bit address

*/

class Address {
public:
	Address(u16 location);
	explicit Address(const RegisterPair& from);
	explicit Address(const WordRegister& from);


	// Get underlying 16-bit value.
	u16 value() const;

	// True if address is between [low, high] inclusive.
	bool in_range(Address low, Address high) const;

	// Compare against a raw 16-bit val.
	bool operator==(u16 other) const;

	Address operator+(uint other) const;
	Address operator-(uint other) const;

private:
	u16 addr;
};