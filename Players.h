//
// Created by David Doellstedt on 5/31/20.
//

#ifndef DDS_CHESS_ENGINE_PLAYERS_H
#define DDS_CHESS_ENGINE_PLAYERS_H
#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;



class Player {

public:
    bool color;
    bool human;
    int difficulty;
    int max_depth;

    bool assign_color();

    Player(bool h): human(h) {  //human player created, random color
        color = assign_color();
    }
    Player(bool h, bool c): human(h), color(c) {} //human player created, determined color
    Player(bool h, int dif, int depth): human(h), difficulty(dif), max_depth(depth) { //AI player created, random color
        color = assign_color();
    }
    Player(bool h, bool c, int dif, int depth): human(h), color(c), difficulty(dif), max_depth(depth) {} //AI player created, determined color

    string get_move();
};



#endif //DDS_CHESS_ENGINE_PLAYERS_H
