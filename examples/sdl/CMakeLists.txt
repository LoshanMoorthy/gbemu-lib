find_package(SDL2 REQUIRED)

add_executable(gbemu-sdl main.cpp)

target_link_libraries(gbemu-sdl
    PRIVATE
        gbemu
        SDL2::SDL2
)