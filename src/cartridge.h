 #pragma once

#include <string>
#include <vector>
#include <memory>

#include "cartridge_info.h"
#include "definitions.h"
#include "address.h"
#include "register.h"

class Cartridge {
public:
	Cartridge(std::vector<u8> rom_data,
			  std::vector<u8> ram_data,
			  std::unique_ptr<CartridgeInfo> info);
	virtual ~Cartridge() = default;

	virtual u8 read(const Address& address) const = 0;
	virtual void write(const Address& address, u8 value) = 0;

	const std::vector<u8>& get_cartridge_ram() const;

protected:
	std::vector<u8> rom;
	std::vector<u8> ram;
	std::unique_ptr<CartridgeInfo> cartridge_info;
};

/*
	Factory func that creates the right MBC type
	from the given ROM and optional RAM data.
*/
std::shared_ptr<Cartridge> get_cartridge(const std::vector<u8>& rom_data,
										 const std::vector<u8>& ram_data = {});

class NoMBC : public Cartridge {
public:
	NoMBC(std::vector<u8> rom_data,
		  std::vector<u8> ram_data,
		  std::unique_ptr<CartridgeInfo> info);

	u8 read(const Address& address) const override;
	void write(const Address& address, u8 value) override;
};

class MBC1 : public Cartridge {
public:
	MBC1(std::vector<u8> rom_data,
		 std::vector<u8> ram_data,
		 std::unique_ptr<CartridgeInfo> info);

	u8 read(const Address& address) const override;
	void write(const Address& address, u8 value) override;
private:
	int current_rom_bank = 1;
	int current_ram_bank = 0;
	bool ram_enabled = false;
	
	// Mode: false => ROM banking mode, true => RAM banking mode
	bool banking_mode_select = false;
};

class MBC3 : public Cartridge {
public:
	MBC3(std::vector<u8> rom_data,
		 std::vector<u8> ram_data,
		 std::unique_ptr<CartridgeInfo> info);

	u8 read(const Address& address) const override;
	void write(const Address& address, u8 value) override;
private:
	int current_rom_bank = 1;
	int current_ram_bank = 0;
	bool ram_enabled = false;

	// MBC3 can also map RTC registers instead of RAM banks
	bool using_rtc = false;
};

class MBC5 : public Cartridge {
public:
	MBC5(std::vector<u8> rom_data,
		 std::vector<u8> ram_data,
		 std::unique_ptr<CartridgeInfo> info);
	
	u8 read(const Address& address) const override;
	void write(const Address& address, u8 value) override;
private:
	int current_rom_bank = 1;
	int current_ram_bank = 0;
	bool ram_enabled = false;
};