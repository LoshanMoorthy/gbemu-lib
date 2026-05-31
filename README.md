# gbemu-lib

A Game Boy emulator core written in C++, packaged as a library.
 
Built out of curiosity and the challenge of figuring out how the original hardware actually works at a low level — cycle timing, memory banking, pixel rendering.

![Tetris running through gbemu-lib](Animation.gif)

> Don’t mind my terrible Tetris gameplay - the emulator is doing its best.

If you want to build a frontend, debugger, TAS tool, or other Game Boy-related project without starting from hundreds of opcodes and hardware registers, the library should give you a decent head start.
 
---
 
## Games

Tested with Tetris and Zelda: Link's Awakening. Other games using the supported cartridge types will likely work as well, but haven't been tested extensively yet.

**Supported MBC types:** ROM Only, MBC1, MBC3, MBC5 (for now)
 
---
 
## Usage

Minimal setup to get a game running:

```cpp
#include "gameboy/gameboy.h"
 
gbemu::GameBoy gb;
 
gb.on_frame([](const uint32_t* pixels, int w, int h) {
    // render 160x144 RGBA pixels however you want
});
 
gb.on_input([](gbemu::Button b) -> bool {
    // return true if that button is held
    return false;
});
 
auto err = gb.load_rom("tetris.gb");
if (err != gbemu::Error::None) {
    // handle error
}
 
gb.run(); // blocking, 60fps
```
 
The library has no rendering dependencies.
 
## Hooks
 
Optional hooks for the curious:

```cpp
// Fires after every CPU instruction
gb.on_instruction([](uint16_t pc, uint8_t opcode, gbemu::Registers regs) {
    // build a debugger, disassembler or tracer
});
 
// Fires on every memory write
gb.on_memory_write([](uint16_t addr, uint8_t value) {
    // watchpoints, cheat detection, memory viewer
});
 
// Fires after each scanline (0-143)
gb.on_scanline([](int line, const uint32_t* pixels) {
    // scanline effects, custom palettes
});
 
// Fires when the Game Boy sends a byte via the link cable
gb.on_serial([](uint8_t byte) {
    // printer support, multiplayer
});
 
// Log messages from the emulator
gb.on_log([](gbemu::LogLevel level, const std::string& msg) {
    std::cout << msg << "\n";
});
```

All optional.
 
---
 
## Lifecycle
 
```cpp
gb.pause();
gb.resume();
gb.stop();
 
gb.is_running();
gb.is_paused();
gb.is_loaded();
```
 
---
 
## Error handling
 
```cpp
auto err = gb.load_rom("game.gb");
 
switch (err) {
    case gbemu::Error::None:                 // good to go
    case gbemu::Error::FileNotFound:         // wrong path
    case gbemu::Error::InvalidROM:           // not a valid GB ROM
    case gbemu::Error::UnsupportedCartridge: // MBC not supported
}
```
 
---
 
## Building

Requires CMake 3.20+ and a C++17 compiler.
 
```bash
git clone https://github.com/LoshanMoorthy/gbemu-lib
cd gbemu-lib
cmake -B build
cmake --build build
```
 
SDL2 example:
 
```bash
./build/examples/sdl/gbemu-sdl tetris.gb
```
 
---
 
## How it works
 
The hard parts, since that's the interesting stuff:
 
- **SM83 CPU** — full instruction set, flags, interrupts, halt, EI/DI timing. One instruction per tick, exactly like the real chip.
- **PPU** — scanline renderer cycling through OAM scan, pixel transfer, HBlank and VBlank. Background, window and sprite layers with palette support. The timing here was the trickiest part to get right.
- **MBC banking** — MBC1/3/5 handle ROM and RAM bank switching so larger games can load their data. Each mapper has slightly different behaviour which makes it fun to debug.
- **OAM DMA** — sprite data transfer that the game triggers, handled cycle-accurately.
- **Interrupts** — VBlank, LCD STAT, Timer and Joypad all go through IE/IF and wake the CPU at the right time.

---
 
## TODO
 
- [ ] Sound (APU)
- [ ] Save states
- [ ] Game Boy Color support
- [ ] More MBC types
- [ ] Tests
- [ ] CI/CD

---

## Resources
 
- [Pan Docs](https://gbdev.io/pandocs/) — the Game Boy technical reference
- [izik's opcode table](https://izik1.github.io/gbops/) — opcode encoding
