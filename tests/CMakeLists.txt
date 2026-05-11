find_package(Catch2 3 REQUIRED)

add_executable(gbemu-tests
    test_main.cpp
    test_lifecycle.cpp
    test_loading.cpp
)

target_link_libraries(gbemu-tests
    PRIVATE
        gbemu
        Catch2::Catch2WithMain
)

target_compile_definitions(gbemu-tests PRIVATE
    TEST_ROM_PATH="${CMAKE_CURRENT_SOURCE_DIR}/roms/tetris.gb"
)