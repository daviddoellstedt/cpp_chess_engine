#include "board.h"
#include "constants.h"
#include "helper_functions.h"
#include "log.h"
#include "move.h"
#include "search.h"
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

// TODO: DOCUMENTATION (change location?)
bool stringContains(std::string substr, std::string str) {
  return str.find(substr) != std::string::npos;
}

// TODO: DOCUMENTATION
void checkForAndSetSpecialMoveTypes(const GameState &game_state, const std::string move_str, Move &move) {
  // Check for promotion moves.
  if (move_str.size() == 5) {
    if (move_str[4] == 'q') {
      move.setMoveType(PROMOTION_QUEEN);
    } else if (move_str[4] == 'r') {
      move.setMoveType(PROMOTION_ROOK);
    } else if (move_str[4] == 'n') {
      move.setMoveType(PROMOTION_KNIGHT);
    } else if (move_str[4] == 'b') {
      move.setMoveType(PROMOTION_BISHOP);
    }
    return;
  }

  ColorState player_state = game_state.whites_turn ? game_state.white : game_state.black;
  uint64_t initial_bitboard = move.getInitialBitboard();
  uint64_t final_bitboard = move.getFinalBitboard();

  // Check for castle moves.
  if (initial_bitboard & player_state.king) {
    if (final_bitboard & (initial_bitboard << 2)) {
      move.setMoveType(CASTLE_KINGSIDE);
      return;
    } else if (final_bitboard & (initial_bitboard >> 2)) {
      move.setMoveType(CASTLE_QUEENSIDE);
      return;
    }
  }

  // Check for pawn push 2 moves.
  if ((initial_bitboard & player_state.pawn) &&
      (final_bitboard &
       (game_state.whites_turn ? player_state.pawn << 16 : player_state.pawn >> 16))) {
    move.setMoveType(PAWN_PUSH_2);
  }
}

// TODO: DOCUMENTATION
Move algebraicMoveToInternalMove(std::string move_str, const GameState &game_state) {
  uint8_t y1 = move_str[0] - 'a';
  uint8_t x1 = move_str[1] - '1';
  uint8_t y2 = move_str[2] - 'a';
  uint8_t x2 = move_str[3] - '1';
  Move move = Move(x1, y1, x2, y2);
  checkForAndSetSpecialMoveTypes(game_state, move_str, move);
  return move;
}

// TODO: DOCUMENTATION
void extractAndApplyMoves(std::string input, GameState &game_state) {
  uint8_t n_moves = 0;
  uint8_t start_pos = input.find("moves") + 6;
  input = input.substr(start_pos, input.size() - start_pos) + " ";
 // printBoard(game_state);
  while (stringContains(" ", input)) {
    uint8_t space_pos = input.find(" ") + 1;
    std::string move_str = input.substr(0, space_pos);
    input = input.substr(space_pos, input.size() - space_pos);
    // printAndWriteToLog(move_str);
    applyMove(algebraicMoveToInternalMove(move_str, game_state), game_state);
 //   printBoard(game_state);
  }
}

void handleInput_uci(void) {
  printAndWriteToLog("id name venus");
  printAndWriteToLog("id author David Doellstedt");
  printAndWriteToLog("uciok");
}

void handleInput_isready(void) { printAndWriteToLog("readyok"); }

void handleInput_position(std::string input, GameState &game_state) {
    memset(&game_state, 0, sizeof(GameState));
  std::string fen = fen_standard;
  bool has_moves = stringContains("moves", input);

  if (stringContains("fen", input)) {
    uint8_t start_pos = input.find("fen") + 4;
    uint16_t end_pos = has_moves ? input.find("moves") - 2 : input.size();
    fen = input.substr(start_pos, end_pos - start_pos);
  }
  fenToGameState(fen, game_state);

  if (has_moves) {
    extractAndApplyMoves(input, game_state);
  }
}

void handleInput_go(std::string input, const GameState &game_state) {
  negamaxTuple choice = negamax(game_state, 6);
  printAndWriteToLog("info score cp -1 pv " + choice.move.toString());
  printAndWriteToLog("bestmove " + choice.move.toString());
}

// TODO ADD DOCUMENTATION.
// Add what will not be supported
void UCIHandleInput(std::string input, GameState &game_state) {
  if (input == "quit") {
    exit(1);
  } else if (input == "uci") {
    handleInput_uci();
  } else if (input == "isready") {
    handleInput_isready();
  } else if (stringContains("position", input)) {
    handleInput_position(input, game_state);
  } else if (stringContains("go", input)) {
    handleInput_go(input, game_state);
  } else if (input == "stop") {
    // do nothing.
  } else {
    //printAndWriteToLog("command :'" + input + "' not supported/recognized.");
  }
}

void UCIStart(void) {
  std::ofstream file(fp_log);

  std::string input;
  GameState game_state;

  while (1) {
    getline(std::cin, input);

    if (input.size() > 0) {
      std::ofstream outfile;
      outfile.open(fp_log, std::ios_base::app);
      outfile << ">> " << input << "\n";
      outfile.close();
    }

    UCIHandleInput(input, game_state);
  }
}