cmake_minimum_required(VERSION 3.16)
project(terminal_games CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(games_lib src/tictactoe.cpp src/game2048.cpp src/hangman.cpp src/minesweeper.cpp src/snake.cpp src/dino.cpp src/blackjack.cpp src/tetris.cpp)
target_include_directories(games_lib PUBLIC src)

if(WIN32)
    add_library(tetris_windows_compile OBJECT src/tetris.cpp)
    target_include_directories(tetris_windows_compile PRIVATE src)
endif()

add_executable(games src/main.cpp)
target_link_libraries(games games_lib)
target_sources(games_lib PRIVATE src/intro.cpp)
target_sources(games_lib PRIVATE src/spaceinvaders.cpp)

add_executable(tests tests/tests.cpp)
target_link_libraries(tests games_lib)

enable_testing()
add_test(NAME tests COMMAND tests)
add_executable(tests_minesweeper tests/tests_minesweeper.cpp)
target_link_libraries(tests_minesweeper games_lib)
add_test(NAME minesweeper_tests COMMAND tests_minesweeper)
