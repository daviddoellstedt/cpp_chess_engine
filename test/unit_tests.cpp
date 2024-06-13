#include "../src/bit_fns.h"
#include "../src/board.h"
#include "../src/constants.h"
#include "../src/move_generator.h"
#include <iostream>
#include <stdint.h>
#include <string>

struct PerftTuple {
  std::string fen = "";
  uint8_t depth = 0;
  uint32_t nodes = 0;
};

uint32_t legalMoveGeneratorTest(std::string fen, uint8_t depth){
  GameState game_state;
  fenToGameState(fen, game_state);
  printBoard(game_state);
  uint32_t nodes = 0;

  bool total = true;

  auto start = std::chrono::high_resolution_clock::now();

  perft(nodes, game_state, depth, depth, total);

  auto end = std::chrono::high_resolution_clock::now();

  std::cout << "depth: " << depth + 0
            << ". time elapsed: " << (double)(end - start).count() / 1000000000
            << " s. nodes searched: " << nodes << "." << std::endl;
  std::cout << "NPS: " << nodes / ((double)(end - start).count() / 1000000000)
            << std::endl;
  std::cout << " " << std::endl;

  return nodes;
}

// https://www.chessprogramming.org/Perft_Results.
PerftTuple perft_tests[5] = {
    {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 5, 4865609},
    {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 4,
     4085603},
    {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 6, 11030083},
    {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5,
     15833292},
    {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487},
};

void testAllPerft(void) {
  auto start = std::chrono::high_resolution_clock::now();
  uint32_t total_nodes = 0;
  uint8_t i = 0;

  for (PerftTuple test : perft_tests) {
    uint32_t nodes_explored = 0;
    GameState game_state;
    fenToGameState(test.fen, game_state);
    perft(nodes_explored, game_state, test.depth, test.depth, true);
    total_nodes += nodes_explored;

    if (nodes_explored != test.nodes) {
      std::cout << "Perft " << i + 0 << " failed! Depth: " << test.depth + 0
                << ". Expected nodes: " << test.nodes + 0
                << ", but got: " << nodes_explored + 0 << std::endl;
      break;
    } else {
      std::cout << "Perft " << i + 0 << " has succeeded!" << std::endl;
    }
    i++;
  }
  auto end = std::chrono::high_resolution_clock::now();

  std::cout << "Time elapsed: " << (double)(end - start).count() / 1000000000
            << " s." << std::endl;
  std::cout << "Total nodes searched: " << total_nodes + 0 << "." << std::endl;
  std::cout << "NPS: "
            << (total_nodes + 0) / ((double)(end - start).count() / 1000000000)
            << std::endl;
  return;
}