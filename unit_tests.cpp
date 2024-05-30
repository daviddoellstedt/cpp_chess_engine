
#include "bit_fns.h"
#include <stdint.h>
#include <iostream>
#include <vector>



uint32_t legalMoveGeneratorTest(std::string fen, uint8_t depth){
    GameState gamestate;
    fenToGameState(fen, gamestate);
    print_board(gamestate);
    uint32_t nodes = 0;
    uint32_t cap_counter = 0;

    bool CM = false;
    bool SM = false;
    uint64_t E_P = 0;
    std::vector<Move> moves;
    std::string n = "total";

    auto start = std::chrono::high_resolution_clock::now();

    perft(nodes, cap_counter, gamestate, moves, E_P, CM, SM, depth, depth, n);

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "depth: " << depth << ". time elapsed: "
              << (double)(end - start).count() / 1000000000
              << " s. nodes searched: " << nodes << "." << std::endl;
    std::cout << "NPS: " << nodes / ((double)(end - start).count() / 1000000000)
              << std::endl;
    std::cout << " " << std::endl;

    return nodes;


        std::cout<<"depth "<<depth<<":"<<std::endl;
        std::cout<<"total nodes: "<<nodes<<std::endl;
        std::cout<<"total captureds: "<<cap_counter<<std::endl;
}