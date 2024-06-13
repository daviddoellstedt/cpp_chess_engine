#pragma once

#include "move_generator.h"
#include <stdint.h>
#include <string>

struct AI_return {
  Move move;
  int16_t value = 0;
  uint32_t nodes_searched = 0;
};

AI_return negamax(GameState game_state, uint8_t depth, int8_t color = 1,
                  int16_t alpha = INT16_MIN, int16_t beta = INT16_MAX);

void generate_board(std::string name, uint8_t diff);
