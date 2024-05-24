//
// Created by David Doellstedt on 5/31/20.
//


#include "Players.h"
#include "Board.h"



//todo: needs to take in a board object and return a move
std::string Player:: get_move(){

    int* p = new int[5];




}


bool Player::assign_color() {
    srand(time(nullptr));
    return rand() % 2;
}
