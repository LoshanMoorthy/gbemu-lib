#include "cartridge.h"
#include "cartridge_info.h"
#include "definitions.h"
#include "files.h"
#include "log.h"
#include "address.h"

std::shared_ptr<Cartridge> get_cartridge(const std::vector<u8>& rom_data,
										 const std::vector<u8>& ram_data) {
	// Parse cartinfo
	auto info = get_info(rom_data);
	if (!info) {
		fatal_error("Failed to parse CartridgeInfo from ROM data");
	}

	switch (info->type) {
		case CartridgeType::ROMOnly:
			return std::make_shared<NoMBC>(rom_data, ram_data, std::move(info));
		case CartridgeType::MBC1:
			return std::make_shared<MBC1>(rom_data, ram_data, std::move(info));
		case CartridgeType::MBC3:
			return std::make_shared<MBC3>(rom_data, ram_data, std::move(info));
		case CartridgeType::MBC5:
			return std::make_shared<MBC5>(rom_data, ram_data, std::move(info));
		default:
			log_error("Cartridge type &d not implemented, defaulting to NoMBC", int(info->type));
			return std::make_shared<NoMBC>(rom_data, ram_data, nullptr);
	}
}

Cartridge::Cartridge(std::vector<u8> rom_data,
					 std::vector<u8> ram_data,
					 std::unique_ptr<CartridgeInfo> info)
	: rom(std::move(rom_data))
	, ram(std::move(ram_data))
	, cartridge_info(std::move(info)) {

	log_info("Created Cartridge base: title=%s, type=%s",
			 cartridge_info->title.c_str(),
			 describe(cartridge_info->type).c_str());
}

const std::vector<u8>& Cartridge::get_cartridge_ram() const {
	return ram;
}

NoMBC::NoMBC(std::vector<u8> rom_data,
			 std::vector<u8> ram_data,
			 std::unique_ptr<CartridgeInfo> info)
	: Cartridge(std::move(rom_data), std::move(ram_data), std::move(info)) { }

u8 NoMBC::read(const Address& address) const {
	const u16 addr = address.value();

	// 0x0000..0x7FFF => ROM
	if (addr < 0x8000) {
		if (addr < rom.size()) {
			return rom[addr];
		}
		// If out of range for some reason, returns 0xFF
		return 0xFF;
	}
	// 0xA000..0xBFFF => external cartridge RAM
	else if (addr >= 0xA000 && addr < 0xC000) {
		size_t offset = addr - 0xA000;
		if (offset < ram.size()) {
			return ram[offset];
		}
		return 0xFF;
	}

	return 0xFF;
}

void NoMBC::write(const Address& address, u8 value) { }

MBC1::MBC1(std::vector<u8> rom_data,
		   std::vector<u8> ram_data,
		   std::unique_ptr<CartridgeInfo> info)
	: Cartridge(std::move(rom_data), std::move(ram_data), std::move(info)) {

	current_rom_bank = 1;
	current_ram_bank = 0;
	ram_enabled = false;
	banking_mode_select = false;
}

u8 MBC1::read(const Address& address) const {
	u16 addr = address.value();

	if (addr < 0x4000) {
		return (addr < rom.size()) ? rom[addr] : 0xFF;
	}
	else if (addr < 0x8000) {
		size_t offset = (size_t)current_rom_bank * 0x4000 + (addr - 0x4000);
		return (offset < rom.size()) ? rom[offset] : 0xFF;
	}
	else if (addr >= 0xA000 && addr < 0xC000) {
		if (!ram_enabled || ram.empty()) return 0xFF;
		size_t offset = (size_t)current_ram_bank * 0x2000 + (addr - 0xA000);
		return (offset < ram.size()) ? ram[offset] : 0xFF;
	}

	return 0xFF;
}

void MBC1::write(const Address& address, u8 value) {
	u16 addr = address.value();

	// Enable/disable RAM => 0x0000..0x1FFF
	if (addr < 0x2000) {
		ram_enabled = ((value & 0x0F) == 0x0A);
	}
	// ROM Bank select => 0x2000..0x3FFF
	else if (addr < 0x4000) {
		int bank_val = (value & 0x1F);
		if (bank_val == 0) bank_val = 1;

		current_rom_bank = (current_rom_bank & 0x60) | bank_val;  // 0x60 = bits 5 and 6
		if (current_rom_bank == 0) {
			current_rom_bank = 1;
		}
	}
	// Upper ROM bank bits or RAM bank select => 0x4000..0x5FFF
	else if (addr < 0x6000) {
		int bank_val = (value & 0x03); // only 2 bits used
		if (!banking_mode_select) {
			int upper_bits = (bank_val << 5);
			int lower_five = (current_rom_bank & 0x1F);
			current_rom_bank = lower_five | upper_bits;
			if ((current_rom_bank & 0x1F) == 0) {
				current_rom_bank = (current_rom_bank & 0xE0) | 1;
			}
		}
		else {
			// In RAM banking mode, these 2 bits select the RAM bank
			current_ram_bank = bank_val;
		}
	}
	// Banking mode select => 0x6000..0x7FFF
	else if (addr < 0x8000) {
		// 0 => ROM banking mode, 1 => RAM banking mode
		banking_mode_select = (value & 0x01);
		if (!banking_mode_select) {
		}
	}
	// Cartridge RAM => 0xA000..0xBFFF
	else if (addr >= 0xA000 && addr < 0xC000) {
		if (ram_enabled && !ram.empty()) {
			size_t offset = addr - 0xA000;
			size_t ram_bank_offset = current_ram_bank * 0x2000;
			size_t final_addr = ram_bank_offset + offset;
			if (final_addr < ram.size()) {
				ram[final_addr] = value;
			}
		}
	}
}

MBC3::MBC3(std::vector<u8> rom_data,
		   std::vector<u8> ram_data,
		   std::unique_ptr<CartridgeInfo> info)
	: Cartridge(std::move(rom_data), std::move(ram_data), std::move(info)) {
	current_rom_bank = 1;
	current_ram_bank = 0;
	ram_enabled = false;
	using_rtc = false;
}

u8 MBC3::read(const Address& address) const {
	u16 addr = address.value();
	if (addr < 0x4000) {
		return (addr < rom.size()) ? rom[addr] : 0xFF;
	}
	else if (addr < 0x8000) {
		size_t bank_offset = (current_rom_bank - 1) * 0x4000;
		size_t offset_in_bank = addr - 0x4000;
		size_t final_addr = bank_offset + offset_in_bank;
		return (final_addr < rom.size()) ? rom[final_addr] : 0xFF;
	}
	else if (addr >= 0xA000 && addr < 0xC000) {
		// Cartridge RAM or RTC register
		if (!ram_enabled) {
			return 0xFF;
		}
		if (!using_rtc) {
			// Normal RAM
			size_t ram_bank_offset = current_ram_bank * 0x2000;
			size_t offset = addr - 0xA000;
			size_t final_addr = ram_bank_offset + offset;
			return (final_addr < ram.size()) ? ram[final_addr] : 0xFF;
		}
		else {
			return 0;
		}
	}
	return 0xFF;
}

void MBC3::write(const Address& address, u8 value) {
	u16 addr = address.value();

	if (addr < 0x2000) {
		// Enable/disable RAM
		ram_enabled = ((value & 0x0F) == 0x0A);
	}
	else if (addr < 0x4000) {
		// ROM bank select (7 bits, 0 => 1)
		int bank_val = (value & 0x7F);
		if (bank_val == 0) bank_val = 1;
		current_rom_bank = bank_val;
	}
	else if (addr < 0x6000) {
		// RAM bank or RTC select
		int bank_val = value & 0x0F;
		if (bank_val <= 0x03) {
			// RAM bank
			current_ram_bank = bank_val;
			using_rtc = false;
		}
		else if (bank_val >= 0x08 && bank_val <= 0x0C) {
			// RTC registers
			using_rtc = true;
		}
	}
	else if (addr < 0x8000) {
		// 0x6000..0x7FFF => Latch clock data, etc. (if implemented)
	}
	else if (addr >= 0xA000 && addr < 0xC000) {
		// Write to RAM or RTC
		if (!ram_enabled) return;

		if (!using_rtc) {
			// Normal RAM write
			size_t offset = addr - 0xA000;
			size_t ram_bank_offset = current_ram_bank * 0x2000;
			size_t final_addr = ram_bank_offset + offset;
			if (final_addr < ram.size()) {
				ram[final_addr] = value;
			}
		}
		else {
		}
	}
}

MBC5::MBC5(std::vector<u8> rom_data,
		   std::vector<u8> ram_data,
		   std::unique_ptr<CartridgeInfo> info)
	: Cartridge(std::move(rom_data), std::move(ram_data), std::move(info)) {
	current_rom_bank = 1;
	current_ram_bank = 0;
	ram_enabled = false;
}

u8 MBC5::read(const Address& address) const {
	u16 addr = address.value();
	if (addr < 0x4000) {
		return (addr < rom.size()) ? rom[addr] : 0xFF;
	}
	else if (addr < 0x8000) {
		size_t offset = (size_t)current_rom_bank * 0x4000 + (addr - 0x4000);
		return (offset < rom.size()) ? rom[offset] : 0xFF;
	}
	else if (addr >= 0xA000 && addr < 0xC000) {
		if (!ram_enabled || ram.empty()) return 0xFF;
		size_t offset = (size_t)current_ram_bank * 0x2000 + (addr - 0xA000);
		return (offset < ram.size()) ? ram[offset] : 0xFF;
	}
	return 0xFF;
}

void MBC5::write(const Address& address, u8 value) {
	u16 addr = address.value();
	if (addr < 0x2000) {
		ram_enabled = ((value & 0x0F) == 0x0A);
	}
	else if (addr < 0x3000) {
		// Low 8 bits of ROM bank
		current_rom_bank = (current_rom_bank & 0x100) | value;
	}
	else if (addr < 0x4000) {
		// Bit 8 of ROM bank
		current_rom_bank = (current_rom_bank & 0xFF) | ((value & 0x01) << 8);
	}
	else if (addr < 0x6000) {
		current_ram_bank = value & 0x0F;
	}
	else if (addr >= 0xA000 && addr < 0xC000) {
		if (!ram_enabled || ram.empty()) return;
		size_t offset = (size_t)current_ram_bank * 0x2000 + (addr - 0xA000);
		if (offset < ram.size()) ram[offset] = value;
	}
}