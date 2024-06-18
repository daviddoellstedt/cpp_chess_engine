cmake_minimum_required(VERSION 3.29)
project(venus_chess)

set(CMAKE_CXX_STANDARD 20)

set(SOURCE_FILES
src/main.cpp
src/uci.cpp
src/search.cpp
src/board.cpp
src/helper_functions.cpp
src/move_generator.cpp
src/evaluate.cpp
test/test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})