#include "bit_fns.h"
#include "board.h"
#include "constants.h"
#include "helper_functions.h"
#include "move.h"
#include <iostream>
#include <string>
#include <unistd.h>

// https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf

// TODO:(change location?)
std::string fen_standard =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// TODO: DOCUMENTATION (change location?)
inline void printStandard(std::string str) { std::cout << str << std::endl; }

// TODO: DOCUMENTATION (change location?)
bool stringContains(std::string substr, std::string str) {
  return str.find(substr) != std::string::npos;
}

// TODO: DOCUMENTATION
Move convertAlgebraicMoveToInternalMove(std::string move_str) {
  uint8_t y1 = move_str[0] - 'a';
  uint8_t x1 = move_str[1] - '1';
  uint8_t y2 = move_str[2] - 'a';
  uint8_t x2 = move_str[3] - '1';
  Move move = Move(x1, y1, x2, y2);

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
  }
  return move;
}

// TODO: DOCUMENTATION
void checkForAndSetSpecialMoveType(const ColorState &player_state,
                                   bool white_to_move, Move &move) {
  uint64_t initial_bitboard = move.getInitialBitboard();
  uint64_t final_bitboard = move.getFinalBitboard();

  // Check for castle moves.
  if (initial_bitboard & player_state.king) {
    if (final_bitboard & (initial_bitboard << 2)) {
      move.setMoveType(CASTLE_KINGSIDE);
    } else if (final_bitboard & (initial_bitboard >> 2)) {
      move.setMoveType(CASTLE_QUEENSIDE);
    }
  }

  // Check for pawn push 2 moves.
  if ((initial_bitboard & player_state.pawn) &&
      (final_bitboard &
       (white_to_move ? player_state.pawn << 16 : player_state.pawn >> 16))) {
    move.setMoveType(PAWN_PUSH_2);
  }
}

// TODO: DOCUMENTATION
void extractAndApplyMoves(std::string input, GameState &game_state) {
  uint8_t n_moves = 0;
  uint8_t start_pos = input.find("moves") + 6;
  input = input.substr(start_pos, input.size() - start_pos) + " ";
  printBoard(game_state);
  while (stringContains(" ", input)) {
    uint8_t space_pos = input.find(" ") + 1;
    std::string move_str = input.substr(0, space_pos);
    input = input.substr(space_pos, input.size() - space_pos);
    // printStandard(move_str);
    Move move = convertAlgebraicMoveToInternalMove(move_str);
    // check for special moves.
    checkForAndSetSpecialMoveType(game_state.whites_turn ? game_state.white
                                                         : game_state.black,
                                  game_state.whites_turn, move);
    applyMove(move, game_state);
    printBoard(game_state);
  }
}

// * uci
// 	tell engine to use the uci (universal chess interface),
// 	this will be sent once as a first command after program boot
// 	to tell the engine to switch to uci mode.
// 	After receiving the uci command the engine must identify itself with the
// "id" command 	and send the "option" commands to tell the GUI which engine
// settings the engine supports if any. 	After that the engine should send
// "uciok" to acknowledge the uci mode. 	If no uciok is sent within a certain
// time period, the engine task will be killed by the GUI.

// TODO ADD DOCUMENTATION.
// Add what will not be supported
void UCIHandleInput(std::string input, GameState &game_state) {
  if (input == "uci") {
    printStandard("id name venus");
    printStandard("id author David Doellstedt");
    printStandard("uci ok");
    return;
  }
  if (input == "debug on") {
    // TODO: implement.
    return;
  }
  if (input == "debug off") {
    // TODO: implement.
    return;
  }
  if (input == "isready") {
    printStandard("readyok");
    return;
  }
  if (stringContains("position", input)) {
    std::string fen = "fen_standard";
    bool has_moves = stringContains("moves", input);

    if (stringContains("fen", input)) {
      uint8_t start_pos = input.find("fen") + 4;
      uint16_t end_pos = has_moves ? input.find("moves") - 2 : input.size();
      fen = input.substr(start_pos, end_pos - start_pos);
    }

    fenToGameState(fen, game_state);

    if (has_moves) {
      // TODO, since I track game state, no need to replay all the moves. If
      // already up to date, just apply the most recent move.
      extractAndApplyMoves(input, game_state);
    }
    return;
  }
  if (stringContains("go", input)) {
    // get negamax move.
    AI_return choice = negamax(game_state, 3, game_state.whites_turn ? 1 : -1);
    printStandard(choice.move.toString());
    return;
  }
  if (input == "") {
  }
  if (input == "") {

  } else {
    std::cout << "FAIL" << std::endl;
  }
}

void UCIStart(void) {
  std::string input;
  GameState game_state;
  while (true) {
    getline(std::cin, input);
    UCIHandleInput(input, game_state);
  }
}