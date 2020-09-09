//
// Created by David Doellstedt on 5/31/20.
//

#ifndef DDS_CHESS_ENGINE_BOARD_H
#define DDS_CHESS_ENGINE_BOARD_H

#include <string>
using namespace std;


//to do: implement FEN functionality
class Board {

public:
    string FEN;
    unsigned long long WP;
    unsigned long long WR;
    unsigned long long WN;
    unsigned long long WB;
    unsigned long long WQ;
    unsigned long long WK;
    unsigned long long BP;
    unsigned long long BR;
    unsigned long long BN;
    unsigned long long BB;
    unsigned long long BQ;
    unsigned long long BK;

    Board(string FEN);
};


#endif //DDS_CHESS_ENGINE_BOARD_H
