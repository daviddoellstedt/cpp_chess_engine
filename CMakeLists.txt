cmake_minimum_required(VERSION 3.16)
project(DDs_chess_engine)

set(CMAKE_CXX_STANDARD 14)

add_executable(DDs_chess_engine main.cpp bit_fns.h bit_fns.cpp Players.h Board.cpp Board.h Players.cpp Helper_functions.cpp Helper_functions.h)