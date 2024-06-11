#pragma once

#include "board.h"
#include "constants.h"
#include "helper_functions.h"
#include <stdint.h>
#include <string>

// struct Move {

//   uint16_t data = 0;
// };

void generate_board(std::string name, uint8_t diff);

void perft(uint32_t &nodes, GameState &game_state, uint8_t depth,
           uint8_t orig_depth, bool total);
