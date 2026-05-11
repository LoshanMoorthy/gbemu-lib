#include <catch2/catch_all.hpp>
#include "gameboy/gameboy.h"

TEST_CASE("is_running returns false before run") {
    gbemu::GameBoy gb;
    REQUIRE(gb.is_running() == false);
}

TEST_CASE("is_paused returns false initially") {
    gbemu::GameBoy gb;
    REQUIRE(gb.is_paused() == false);
}

TEST_CASE("cannot run without loading a ROM") {
    gbemu::GameBoy gb;
    gb.on_log([](gbemu::LogLevel, const std::string&) {});
    gb.run();
    REQUIRE(gb.is_running() == false);
}

TEST_CASE("pause before running does nothing") {
    gbemu::GameBoy gb;
    gb.pause();
    REQUIRE(gb.is_paused() == false);
    REQUIRE(gb.is_running() == false);
}

TEST_CASE("resume when not paused does nothing") {
    gbemu::GameBoy gb;
    gb.resume();
    REQUIRE(gb.is_paused() == false);
}

TEST_CASE("stop before running does nothing") {
    gbemu::GameBoy gb;
    gb.stop();
    REQUIRE(gb.is_running() == false);
}

TEST_CASE("stop sets is_running to false") {
    gbemu::GameBoy gb;
    gb.stop();
    REQUIRE(gb.is_running() == false);
}

TEST_CASE("on_log callback fires") {
    gbemu::GameBoy gb;
    bool fired = false;
    gb.on_log([&](gbemu::LogLevel, const std::string&) {
        fired = true;
    });
    gb.load_rom(TEST_ROM_PATH);
    REQUIRE(fired == true);
}

TEST_CASE("configure before load applies correctly") {
    gbemu::GameBoy gb;
    gbemu::Config config;
    config.enable_logging = false;
    gb.configure(config);
    bool log_fired = false;
    gb.on_log([&](gbemu::LogLevel, const std::string&) {
        log_fired = true;
    });
    gb.load_rom(TEST_ROM_PATH);
    REQUIRE(gb.is_loaded() == true);
}

TEST_CASE("version numbers are set") {
    REQUIRE(gbemu::GameBoy::version_major() == 0);
    REQUIRE(gbemu::GameBoy::version_minor() == 1);
    REQUIRE(gbemu::GameBoy::version_patch() == 0);
}