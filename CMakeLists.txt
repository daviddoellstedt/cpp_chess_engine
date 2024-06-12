cmake_minimum_required(VERSION 3.29)
project(cpp_chess_engine)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES
src/main.cpp
src/bit_fns.cpp
src/board.cpp
src/helper_functions.cpp
src/player.cpp
src/move_generator.cpp
test/unit_tests.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})