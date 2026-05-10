#include "gameboy/gameboy.h"

#include <SDL2/SDL.h>
#include <iostream>

using std::cout;
using std::cerr;
using std::string;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: gbemu-sdl <rom.gb>\n";
        return 1;
    }

    // -- Setup SDL --
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cerr << "SDl_Init failed: " << SDL_GetError() << "\n";
        return 1;
    }

    const int SCALE = 4;
    SDL_Window* window = SDL_CreateWindow(
        "gbemu-lib",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        160 * SCALE, 144 * SCALE,
        SDL_WINDOW_SHOWN
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture* texture   = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        160, 144
    );

    // -- Setup Emulator --
    gbemu::GameBoy gb;

    gb.on_log([](gbemu::LogLevel level, const std::string& msg) {
        switch (level) {
            case gbemu::LogLevel::Error:   std::cerr << "[ERROR] " << msg << "\n"; break;
            case gbemu::LogLevel::Warning: std::cerr << "[WARN]  " << msg << "\n"; break;
            case gbemu::LogLevel::Info:    std::cout << "[INFO]  " << msg << "\n"; break;
        }
    });

    auto err = gb.load_rom(argv[1]);
    if (err != gbemu::Error::None) {
        std::cerr << "Failed to load ROM\n";
        SDL_Quit();
        return 1;
    }

    gb.on_frame([&](const uint32_t* pixels, int w, int h) {
        SDL_UpdateTexture(texture, nullptr, pixels, w * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    });

    gb.on_input([&](gbemu::Button button) -> bool {
        const uint8_t* keys = SDL_GetKeyboardState(nullptr);
        switch (button) {
            case gbemu::Button::A:      return keys[SDL_SCANCODE_Z];
            case gbemu::Button::B:      return keys[SDL_SCANCODE_X];
            case gbemu::Button::Start:  return keys[SDL_SCANCODE_RETURN];
            case gbemu::Button::Select: return keys[SDL_SCANCODE_RSHIFT] || keys[SDL_SCANCODE_LSHIFT];
            case gbemu::Button::Up:     return keys[SDL_SCANCODE_UP];
            case gbemu::Button::Down:   return keys[SDL_SCANCODE_DOWN];
            case gbemu::Button::Left:   return keys[SDL_SCANCODE_LEFT];
            case gbemu::Button::Right:  return keys[SDL_SCANCODE_RIGHT];
            default:                    return false;
        }
    });

    gb.on_tick([&]() {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) gb.stop();
            if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
                if (e.key.keysym.sym == SDLK_ESCAPE) gb.stop();
            }
        }
    });

    // -- Run --
    gb.run();

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}