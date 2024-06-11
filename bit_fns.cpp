#include "bit_fns.h"
#include "Players.h"
#include "board.h"
#include "constants.h"
#include "helper_functions.h"
#include "move_generator.h"
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

void printBitboard(uint64_t bb) {
  for (int i = 56; i >= 0; i -= 8) {
    std::bitset<8> bb_bitset((bb >> i) & 0xFF);
    std::cout << bb_bitset << std::endl;
  }
}

uint64_t moveGetInitialPositionBitboard(Move move) {
  return (uint64_t)1 << ((move.getX1() * 8) + (move.getY1() % 8));
}

uint64_t moveGetFinalPositionBitboard(Move move) {
  return (uint64_t)1 << ((move.getX2() * 8) + (move.getY2() % 8));
}

void handleCapturedPiece(bool white_to_move, uint64_t P,
                         PlayerState &enemy_player, uint64_t E_P,
                         uint64_t initial, uint64_t final) {
  uint64_t ENEMY_PIECES = white_to_move
                              ? generatePlayerOccupiedBitboard(enemy_player)
                              : generatePlayerOccupiedBitboard(enemy_player);

  if (ENEMY_PIECES & final) {
    if (enemy_player.pawn & final) {
      enemy_player.pawn &= ~final;
      return;
    }
    if (enemy_player.knight & final) {
      enemy_player.knight &= ~final;
      return;
    }
    if (enemy_player.bishop & final) {
      enemy_player.bishop &= ~final;
      return;
    }
    if (enemy_player.queen & final) {
      enemy_player.queen &= ~final;
      return;
    }
    if (enemy_player.rook & final) {
      enemy_player.rook &= ~final;
      if (final & (white_to_move ? BLACK_ROOK_STARTING_POSITION_KINGSIDE
                                 : WHITE_ROOK_STARTING_POSITION_KINGSIDE)) {
        enemy_player.can_king_side_castle = false;
      } else if (final &
                 (white_to_move ? BLACK_ROOK_STARTING_POSITION_QUEENSIDE
                                : WHITE_ROOK_STARTING_POSITION_QUEENSIDE)) {
        enemy_player.can_queen_side_castle = false;
      }
      return;
    }
  }
  if ((E_P & final) && (P & initial)) {
    enemy_player.pawn &= (white_to_move ? ~(final >> 8) : ~(final << 8));
    return;
  }
}

void realizeMovedPiece(bool white_to_move, PlayerState &active_player,
                       uint64_t &E_P, uint64_t initial, uint64_t final,
                       SpecialMove special) {
  switch (special) {
  case NONE:
    if (active_player.queen & initial) {
      active_player.queen |= final;
      active_player.queen &= ~initial;
    } else if (active_player.bishop & initial) {
      active_player.bishop |= final;
      active_player.bishop &= ~initial;
    } else if (active_player.knight & initial) {
      active_player.knight |= final;
      active_player.knight &= ~initial;
    } else if (active_player.pawn & initial) {
      active_player.pawn |= final;
      active_player.pawn &= ~initial;
    } else if (active_player.king & initial) {
      active_player.can_king_side_castle = false;
      active_player.can_queen_side_castle = false;
      active_player.king = final;
    } else if (active_player.rook & initial) {
      if (initial & (white_to_move ? WHITE_ROOK_STARTING_POSITION_KINGSIDE
                                   : BLACK_ROOK_STARTING_POSITION_KINGSIDE)) {
        active_player.can_king_side_castle = false;
      } else if (initial &
                 (white_to_move ? WHITE_ROOK_STARTING_POSITION_QUEENSIDE
                                : BLACK_ROOK_STARTING_POSITION_QUEENSIDE)) {
        active_player.can_queen_side_castle = false;
      }
      active_player.rook |= final;
      active_player.rook &= ~initial;
    }
    E_P = 0;
    return;
  case CASTLE_KINGSIDE:
    active_player.king <<= 2;
    active_player.rook |= white_to_move
                              ? WHITE_ROOK_POST_KINGSIDE_CASTLE_POSITION
                              : BLACK_ROOK_POST_KINGSIDE_CASTLE_POSITION;
    active_player.rook &= white_to_move
                              ? ~WHITE_ROOK_STARTING_POSITION_KINGSIDE
                              : ~BLACK_ROOK_STARTING_POSITION_KINGSIDE;
    active_player.can_king_side_castle = false;
    active_player.can_queen_side_castle = false;
    E_P = 0;
    return;
  case CASTLE_QUEENSIDE:
    active_player.king >>= 2;
    active_player.rook |= white_to_move
                              ? WHITE_ROOK_POST_QUEENSIDE_CASTLE_POSITION
                              : BLACK_ROOK_POST_QUEENSIDE_CASTLE_POSITION;
    active_player.rook &= white_to_move
                              ? ~WHITE_ROOK_STARTING_POSITION_QUEENSIDE
                              : ~BLACK_ROOK_STARTING_POSITION_QUEENSIDE;
    active_player.can_king_side_castle = false;
    active_player.can_queen_side_castle = false;
    E_P = 0;
    return;
  case PROMOTION_QUEEN:
    active_player.pawn &= ~initial;
    active_player.queen |= final;
    E_P = 0;
    return;
  case PROMOTION_ROOK:
    active_player.pawn &= ~initial;
    active_player.rook |= final;
    E_P = 0;
    return;
  case PROMOTION_BISHOP:
    active_player.pawn &= ~initial;
    active_player.bishop |= final;
    E_P = 0;
    return;
  case PROMOTION_KNIGHT:
    active_player.pawn &= ~initial;
    active_player.knight |= final;
    E_P = 0;
    return;
  case EN_PASSANT:
    active_player.pawn |= final;
    active_player.pawn &= ~initial;
    E_P = 0;
    return;
  case PAWN_PUSH_2:
    active_player.pawn |= final;
    active_player.pawn &= ~initial;
    E_P = white_to_move ? final >> 8 : final << 8;
    return;
  default:
    logErrorAndExit("ERROR: Unexpected SpecialMove value!");
    return;
  }
}

void applyWhiteMove(GameState &gamestate, const Move &move, uint64_t initial,
                    uint64_t final, SpecialMove special) {
  handleCapturedPiece(gamestate.whites_turn, gamestate.white.pawn,
                      gamestate.black, gamestate.en_passant, initial, final);
  realizeMovedPiece(gamestate.whites_turn, gamestate.white,
                    gamestate.en_passant, initial, final, special);
}

void applyBlackMove(GameState &gamestate, const Move &move, uint64_t initial,
                    uint64_t final, SpecialMove special) {
  handleCapturedPiece(gamestate.whites_turn, gamestate.black.pawn,
                      gamestate.white, gamestate.en_passant, initial, final);
  realizeMovedPiece(gamestate.whites_turn, gamestate.black,
                    gamestate.en_passant, initial, final, special);
}

void applyMove(Move move, GameState &gamestate) {
  const uint64_t initial = moveGetInitialPositionBitboard(move);
  const uint64_t final = moveGetFinalPositionBitboard(move);
  const SpecialMove special = move.getSpecial();
  if (gamestate.whites_turn) {
    applyWhiteMove(gamestate, move, initial, final, special);
  } else {
    applyBlackMove(gamestate, move, initial, final, special);
  }
  gamestate.whites_turn = !gamestate.whites_turn;
}

void print_moves(bool white_to_move, Move *moves, uint8_t n_moves) {
  std::cout << (white_to_move ? "WHITE" : "BLACK") << "'S MOVE: " << std::endl;
  for (uint8_t i = 0; i < n_moves; i++) {
    std::cout << i + 1 << ": " + moves[i].toString() << std::endl;
  }
}

void perft(uint32_t &nodes, GameState &gamestate, uint8_t depth,
           uint8_t orig_depth, bool total) {
  bool check = false;
  Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];
  uint8_t n_moves = generateMoves(gamestate, moves, check);

  if (depth == 1) {
    nodes += n_moves;
  }

  if (depth > 1) {

    for (uint8_t i = 0; i < n_moves; i++) {
      GameState gamestate_temp;
      memcpy(&gamestate_temp, &gamestate, sizeof(GameState));
      applyMove(moves[i], gamestate_temp);

      perft(nodes, gamestate_temp, uint8_t(depth - 1), orig_depth, total);

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

int16_t eval(const GameState gamestate) {
  // material
  int16_t counter = 0;
  counter += (countSetBits(gamestate.white.pawn) -
              countSetBits(gamestate.black.pawn)) *
             100;
  counter += (countSetBits(gamestate.white.bishop) -
              countSetBits(gamestate.black.bishop)) *
             300; // todo: add special case regarding number of bishops
  counter += (countSetBits(gamestate.white.knight) -
              countSetBits(gamestate.black.knight)) *
             300;
  counter += (countSetBits(gamestate.white.rook) -
              countSetBits(gamestate.black.rook)) *
             500;
  counter += (countSetBits(gamestate.white.queen) -
              countSetBits(gamestate.black.queen)) *
             900;
  //    if (counter > 1000000){
  //       std::cout << counter << std::endl;
  //       printBoard(gamestate);
  //       exit(1);
  //    }

  return counter;
}

AI_return negamax(GameState gamestate, uint8_t depth, int8_t color = 1,
                  int16_t alpha = INT16_MIN, int16_t beta = INT16_MAX) {
  AI_return node_max;
  node_max.value = INT16_MIN;
  node_max.nodes_searched++;

  // Terminal Node.
  if (depth == 0) {
    Move leaf_move;
    AI_return leaf = {leaf_move, (int16_t)(eval(gamestate) * color), 1};
    return leaf;
  }

  bool check = false;
  Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];
  uint8_t n_moves = generateMoves(gamestate, moves, check);

  // Terminal node, Checkmate/Stalemate.
  if (n_moves == 0) {
    AI_return leaf = {Move(), (int16_t)(check ? INT16_MAX * -color : 0), 1};
    return leaf;
  }

  for (uint8_t i = 0; i < n_moves; i++) {
    GameState gamestate_temp;
    memcpy(&gamestate_temp, &gamestate, sizeof(GameState));
    applyMove(moves[i], gamestate_temp);
    AI_return node_temp =
        negamax(gamestate_temp, depth - 1, -color, -beta, -alpha);
    // AI_return node_temp =
    //     negamax(move_gamestate_scores[i].gamestate, depth - 1, -color, -beta,
    //     -alpha);
    node_temp.value *= -color;
    node_max.nodes_searched += node_temp.nodes_searched;

    if (node_temp.value > node_max.value) {
      node_max.value = node_temp.value;
      node_max.move = moves[i];
      // node_max.move = move_gamestate_scores[i].move;
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

  GameState gamestate;
  fenToGameState(FEN, gamestate);

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

    uint64_t WHITE_PIECES = generateWhiteOccupiedBitboard(gamestate);
    uint64_t BLACK_PIECES = generateBlackOccupiedBitboard(gamestate);
    uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    if (gamestate.whites_turn) {

      std::cout << "WHITE'S MOVE: " << std::endl;
      std::cout << "AI Agent thinking... wait a few seconds." << std::endl;
      auto start = std::chrono::high_resolution_clock::now();
      depth = 8;
      AI_choice = negamax(gamestate, depth);
      std::cout << AI_choice.nodes_searched + 0 << std::endl;

      auto end = std::chrono::high_resolution_clock::now();

      std::cout << "Move chosen: " << AI_choice.move.toString() << std::endl;
      std::cout << AI_choice.value << std::endl;

      applyMove(AI_choice.move, gamestate);

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
      printBoard(gamestate);
      std::cout << "BLACK'S MOVE: " << std::endl;

      // todo: create a player class for their choosing mechanism
      Move moves[MAX_POSSIBLE_MOVES_PER_POSITION];

      // TODO: uncomment this and fix
      bool check = false;
      uint8_t n_moves = generateMoves(gamestate, moves, check);

      std::cout << "Please select your move: " << std::endl;
      print_moves(gamestate.whites_turn, moves, n_moves);

      int user_choice;
      std::cin >> user_choice;

      applyMove(moves[user_choice - 1], gamestate);

      std::cout << "Move chosen: " << moves[user_choice - 1].toString()
                << std::endl;
      std::cout << " " << std::endl;
    }
  }
}
