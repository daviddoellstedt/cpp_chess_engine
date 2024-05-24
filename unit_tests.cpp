
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
    std::vector<std::string> moves;
    std::string n = "total";




    perft(nodes, cap_counter, gamestate, moves, E_P, CM, SM, depth, depth, n);

    
    return nodes;


        std::cout<<"depth "<<depth<<":"<<std::endl;
        std::cout<<"total nodes: "<<nodes<<std::endl;
        std::cout<<"total captureds: "<<cap_counter<<std::endl;
}