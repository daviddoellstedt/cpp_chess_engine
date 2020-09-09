//
// Created by David Doellstedt on 5/31/20.
//


#include "Players.h"
#include "Board.h"

using namespace std;


//todo: needs to take in a board object and return a move
string Player:: get_move(){

}


bool Player::assign_color() {
    srand(time(nullptr));
    return rand() % 2;
}
