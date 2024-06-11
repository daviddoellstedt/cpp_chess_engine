#include "bit_fns.h"
#include "board.h"
#include "constants.h"
#include "helper_functions.h"
#include "move.h"
#include "move_generator.h"
#include "player.h"
#include <bitset>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdint.h>
#include <string>

struct AI_return {
  Move move;
  int16_t value = 0;
  uint32_t nodes_searched = 0;
};

// uint64_t moveGetInitialPositionBitboard(Move move) {
//   return (uint64_t)1 << ((move.getX1() * 8) + (move.getY1() % 8));
// }

// uint64_t moveGetFinalPositionBitboard(Move move) {
//   return (uint64_t)1 << ((move.getX2() * 8) + (move.getY2() % 8));
// }

void perft(uint32_t &nodes, GameState &game_state, uint8_t depth,
           uint8_t orig_depth, bool total) {
  bool check = false;
  Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];
  uint8_t n_moves = generateMoves(game_state, moves, check);

  if (depth == 1) {
    nodes += n_moves;
  }

  if (depth > 1) {

    for (uint8_t i = 0; i < n_moves; i++) {
      GameState game_state_temp;
      memcpy(&game_state_temp, &game_state, sizeof(GameState));
      applyMove(moves[i], game_state_temp);

      perft(nodes, game_state_temp, uint8_t(depth - 1), orig_depth, total);

      // TODO: make part of a 'verbose' flag.
      if (depth == orig_depth && false) {
        if (total) {
          std::cout << round(((i * 100 / n_moves)))
                    << "% complete... -> d1:" << moves[i].toString()
                    << "--------------------------------------------------"
                    << std::endl;

        } else { // node based
          std::cout << i << ":" << moves[i].toString() << " " << nodes
                    << std::endl;
          nodes = 0;
        }
      }
    }
  }
}

int16_t eval(const GameState game_state) {
  // material
  int16_t counter = 0;
  counter += (countSetBits(game_state.white.pawn) -
              countSetBits(game_state.black.pawn)) *
             100;
  counter += (countSetBits(game_state.white.bishop) -
              countSetBits(game_state.black.bishop)) *
             300; // todo: add special case regarding number of bishops
  counter += (countSetBits(game_state.white.knight) -
              countSetBits(game_state.black.knight)) *
             300;
  counter += (countSetBits(game_state.white.rook) -
              countSetBits(game_state.black.rook)) *
             500;
  counter += (countSetBits(game_state.white.queen) -
              countSetBits(game_state.black.queen)) *
             900;
  //    if (counter > 1000000){
  //       std::cout << counter << std::endl;
  //       printBoard(game_state);
  //       exit(1);
  //    }

  return counter;
}

AI_return negamax(GameState game_state, uint8_t depth, int8_t color = 1,
                  int16_t alpha = INT16_MIN, int16_t beta = INT16_MAX) {
  AI_return node_max;
  node_max.value = INT16_MIN;
  node_max.nodes_searched++;

  // Terminal Node.
  if (depth == 0) {
    Move leaf_move;
    AI_return leaf = {leaf_move, (int16_t)(eval(game_state) * color), 1};
    return leaf;
  }

  bool check = false;
  Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];
  uint8_t n_moves = generateMoves(game_state, moves, check);

  // Terminal node, Checkmate/Stalemate.
  if (n_moves == 0) {
    AI_return leaf = {Move(), (int16_t)(check ? INT16_MAX * -color : 0), 1};
    return leaf;
  }

  for (uint8_t i = 0; i < n_moves; i++) {
    GameState game_state_temp;
    memcpy(&game_state_temp, &game_state, sizeof(GameState));
    applyMove(moves[i], game_state_temp);
    AI_return node_temp =
        negamax(game_state_temp, depth - 1, -color, -beta, -alpha);
    // AI_return node_temp =
    //     negamax(move_game_state_scores[i].game_state, depth - 1, -color,
    //     -beta, -alpha);
    node_temp.value *= -color;
    node_max.nodes_searched += node_temp.nodes_searched;

    if (node_temp.value > node_max.value) {
      node_max.value = node_temp.value;
      node_max.move = moves[i];
      // node_max.move = move_game_state_scores[i].move;
    }

    alpha = std::max(alpha, node_max.value);
    if (false or alpha >= beta) {
      break;
    }
  }
  return node_max;
}

void generate_board(std::string name, uint8_t diff) {
  std::cout << "GAME START" << std::endl;

  std::string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  bool CM = false, SM = false;

  GameState game_state;
  fenToGameState(FEN, game_state);

  AI_return AI_choice;

  uint8_t depth;
  if (diff == 1) {
    depth = 4;
  } else if (diff == 2) {
    depth = 5;
  } else if (diff == 3) {
    depth = 3;
  }

  // for now, the AI is only white
  // todo: implement AI for both colors
  srand(time(nullptr));

  Player p = Player(true);
  Player p2 = Player(true, true);

  while (!CM && !SM) {

    uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(game_state);
    uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(game_state);
    uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    if (game_state.whites_turn) {

      std::cout << "WHITE'S MOVE: " << std::endl;
      std::cout << "AI Agent thinking... wait a few seconds." << std::endl;
      auto start = std::chrono::high_resolution_clock::now();
      depth = 8;
      AI_choice = negamax(game_state, depth);
      std::cout << AI_choice.nodes_searched + 0 << std::endl;

      auto end = std::chrono::high_resolution_clock::now();

      std::cout << "Move chosen: " << AI_choice.move.toString() << std::endl;
      std::cout << AI_choice.value << std::endl;

      applyMove(AI_choice.move, game_state);

      std::cout << "depth: " << depth + 0 << ". time elapsed: "
                << (double)(end - start).count() / 1000000000
                << " s. nodes searched: " << AI_choice.nodes_searched << "."
                << std::endl;
      std::cout << "NPS: "
                << AI_choice.nodes_searched /
                       ((double)(end - start).count() / 1000000000)
                << std::endl;
      std::cout << " " << std::endl;
    } else {
      printBoard(game_state);
      std::cout << "BLACK'S MOVE: " << std::endl;

      // todo: create a player class for their choosing mechanism
      Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];

      // TODO: uncomment this and fix
      bool check = false;
      uint8_t n_moves = generateMoves(game_state, moves, check);

      std::cout << "Please select your move: " << std::endl;
      print_moves(game_state.whites_turn, moves, n_moves);

      int user_choice;
      std::cin >> user_choice;

      applyMove(moves[user_choice - 1], game_state);

      std::cout << "Move chosen: " << moves[user_choice - 1].toString()
                << std::endl;
      std::cout << " " << std::endl;
    }
  }
}
