#include "../src/board.h"
#include "../src/constants.h"
#include "../src/move_generator.h"
#include <iostream>
#include <stdint.h>
#include <string>

struct PerftTuple {
  std::string fen = "";
  int depth = 0;
  int nodes = 0;
};

// https://www.chessprogramming.org/Perft_Results.
PerftTuple perft_tests[6] = {
    {fen_standard, 5, 4865609},
    {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 4,
     4085603},
    {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 6, 11030083},
    {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 5,
     15833292},
    {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 4, 2103487},
    {"rk6/1P6/4q3/8/1Q6/8/4p3/4K3 w - - 0 1", 4, 382695}};

/** Runs the perft test to check accuracy of the move generator.
 *
 * @param nodes: Stores the total number of nodes explored.
 * @param game_state: Game state.
 * @param depth: Depth to test to.
 * @param orig_depth: Depth to test to. Used for printing status.
 * @param total: Flag to denote if the search is for total nodes, or nodes per
 * move at depth 1.
 */
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
    }
  }
}

void testAllPerft(void) {
  auto start = std::chrono::high_resolution_clock::now();
  int total_nodes = 0;
  int i = 0;

  for (PerftTuple test : perft_tests) {
    uint32_t nodes_explored = 0;
    GameState game_state;
    fenToGameState(test.fen, game_state);
    perft(nodes_explored, game_state, test.depth, test.depth, true);
    total_nodes += nodes_explored;

    if (nodes_explored != test.nodes) {
      std::cout << "Perft " << i << " failed! Depth: " << test.depth
                << ". Expected nodes: " << test.nodes
                << ", but got: " << nodes_explored << std::endl;
      break;
    } else {
      std::cout << "Perft " << i << " has succeeded!" << std::endl;
    }
    i++;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "Time elapsed: " << (double)(end - start).count() / 1000000000
            << " s." << std::endl;
  std::cout << "Total nodes searched: " << total_nodes << "." << std::endl;
  std::cout << "NPS: "
            << total_nodes / ((double)(end - start).count() / 1000000000)
            << std::endl;
  return;
}
