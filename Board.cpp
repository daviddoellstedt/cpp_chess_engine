#include "board.h"
#include <iostream>
#include <string>

/** Fills out char grid according to the game state.
 *
 * @param game_state: State of the game.
 * @param grid: Character grid.
 */
void populateGridFromGameState(const GameState &game_state, char grid[8][8]) {
  for (int8_t row = 7; row >= 0; row--) {
    for (uint8_t col = 0; col < 8; col++) {
      uint8_t bit = ((7 - row) * 8) + col;
      if (game_state.white.pawn & (1ULL << bit)) {
        grid[row][col] = 'P';
      } else if (game_state.white.rook & (1ULL << bit)) {
        grid[row][col] = 'R';
      } else if (game_state.white.knight & (1ULL << bit)) {
        grid[row][col] = 'N';
      } else if (game_state.white.bishop & (1ULL << bit)) {
        grid[row][col] = 'B';
      } else if (game_state.white.queen & (1ULL << bit)) {
        grid[row][col] = 'Q';
      } else if (game_state.white.king & (1ULL << bit)) {
        grid[row][col] = 'K';
      } else if (game_state.black.pawn & (1ULL << bit)) {
        grid[row][col] = 'p';
      } else if (game_state.black.rook & (1ULL << bit)) {
        grid[row][col] = 'r';
      } else if (game_state.black.knight & (1ULL << bit)) {
        grid[row][col] = 'n';
      } else if (game_state.black.bishop & (1ULL << bit)) {
        grid[row][col] = 'b';
      } else if (game_state.black.queen & (1ULL << bit)) {
        grid[row][col] = 'q';
      } else if (game_state.black.king & (1ULL << bit)) {
        grid[row][col] = 'k';
      } else {
        grid[row][col] = ' ';
      }
    }
  }
}

void printBoard(const GameState &game_state) {
  char grid[8][8];
  populateGridFromGameState(game_state, grid);

  std::string dividing_line =
      "|---|-----|-----|-----|-----|-----|-----|-----|-----|";

  std::string line;
  for (uint8_t i = 0; i <= 7; i++) {
    std::cout << dividing_line << std::endl;
    line = "| " + std::to_string(8 - i) + " |  ";
    for (uint8_t j = 0; j < 8; j++) {
      line.push_back(grid[i][j]);
      if (j != 7) {
        line += "  |  ";
      }
    }
    std::cout << line + "  |" << std::endl;
  }
  std::cout << dividing_line << std::endl;

  std::cout << "|   |  a  |  b  |  c  |  d  |  e  |  f  |  g  |  h  |"
            << std::endl;
  std::cout << dividing_line << std::endl;
}

uint64_t generatePlayerOccupiedBitboard(const PlayerState &player_state) {
  return player_state.pawn | player_state.rook | player_state.knight |
         player_state.bishop | player_state.queen | player_state.king;
}

uint64_t generateWhiteOccupiedBitboard(const GameState &game_state) {
  return generatePlayerOccupiedBitboard(game_state.white);
}

uint64_t generateBlackOccupiedBitboard(const GameState &game_state) {
  return generatePlayerOccupiedBitboard(game_state.black);
}
