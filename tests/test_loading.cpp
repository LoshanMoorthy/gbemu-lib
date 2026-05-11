#include <catch2/catch_all.hpp>
#include "gameboy/gameboy.h"

TEST_CASE("load_rom returns FileNotFound for missing file") {
    gbemu::GameBoy gb;
    REQUIRE(gb.load_rom("doesnt_exist.gb") == gbemu::Error::FileNotFound);
}

TEST_CASE("load_rom returns FileNotFound for empty path") {
    gbemu::GameBoy gb;
    REQUIRE(gb.load_rom("") == gbemu::Error::FileNotFound);
}

TEST_CASE("load_rom returns None for valid ROM") {
    gbemu::GameBoy gb;
    auto err = gb.load_rom(TEST_ROM_PATH);
    REQUIRE(err == gbemu::Error::None);
}

TEST_CASE("is_loaded returns false before loading") {
    gbemu::GameBoy gb;
    REQUIRE(gb.is_loaded() == false);
}

TEST_CASE("is_loaded returns true after successful load") {
    gbemu::GameBoy gb;
    auto err = gb.load_rom(TEST_ROM_PATH);
    REQUIRE(err == gbemu::Error::None);
    REQUIRE(gb.is_loaded() == true);
}

TEST_CASE("load_rom can be called twice") {
    gbemu::GameBoy gb;
    auto err1 = gb.load_rom(TEST_ROM_PATH);
    auto err2 = gb.load_rom(TEST_ROM_PATH);
    REQUIRE(err1 == gbemu::Error::None);
    REQUIRE(err2 == gbemu::Error::None);
    REQUIRE(gb.is_loaded() == true);
}

TEST_CASE("load_rom can recover after failed load") {
    gbemu::GameBoy gb;
    gb.on_log([](gbemu::LogLevel, const std::string&) {});
    auto err1 = gb.load_rom("doesnt_exist.gb");
    REQUIRE(err1 == gbemu::Error::FileNotFound);
    auto err2 = gb.load_rom(TEST_ROM_PATH);
    REQUIRE(err2 == gbemu::Error::None);
    REQUIRE(gb.is_loaded() == true);
}