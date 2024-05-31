#include "bit_fns.h"
#include <iostream>
#include <stdint.h>
#include <string>
#include <tuple>
#include <vector>

uint32_t legalMoveGeneratorTest(std::string fen, uint8_t depth){
    GameState gamestate;
    fenToGameState(fen, gamestate);
    print_board(gamestate);
    uint32_t nodes = 0;

    bool CM = false;
    bool SM = false;
    std::vector<Move> moves;
    bool total = true;

    auto start = std::chrono::high_resolution_clock::now();

    perft(nodes, gamestate, moves, CM, SM, depth, depth, total);

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "depth: " << depth + 0 << ". time elapsed: "
              << (double)(end - start).count() / 1000000000
              << " s. nodes searched: " << nodes << "." << std::endl;
    std::cout << "NPS: " << nodes / ((double)(end - start).count() / 1000000000)
              << std::endl;
    std::cout << " " << std::endl;

    return nodes;
}

// https://www.chessprogramming.org/Perft_Results.
std::tuple<std::string, uint8_t, uint32_t> perft_test[5] = {
    {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281},
    {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 3,
     97862},
    {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 5, 674624},
    {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 4,
     422333},
    {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379},
};

void testAllPerft(void) {
  auto start = std::chrono::high_resolution_clock::now();
  uint32_t total_nodes = 0;
  uint8_t i = 0;

  for (std::tuple<std::string, uint8_t, uint32_t> test : perft_test) {
    std::string fen = get<0>(test);
    uint8_t depth = get<1>(test);
    uint32_t nodes_expected = get<2>(test);
    uint32_t nodes = 0;
    GameState gamestate;
    fenToGameState(fen, gamestate);
    bool CM = false;
    bool SM = false;
    std::vector<Move> moves;

    perft(nodes, gamestate, moves, CM, SM, depth, depth, "total");

    total_nodes += nodes;

    if (nodes != nodes_expected) {
      std::cout << "Perft " << i + 0 << " failed! Depth: " << depth + 0
                << ". Expected nodes: " << nodes_expected + 0
                << ", but got: " << nodes + 0 << std::endl;
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