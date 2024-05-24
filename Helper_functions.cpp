//
// Created by David Doellstedt on 5/31/20.
//

#include "Helper_functions.h"
#include <bitset>
#include <iostream>
#include <cmath>
#include <stdint.h>


//todo: Need to make this more efficient --> LATER
// maybe can use the bitset find next function??
void ind_bbs(uint64_t bb, std::vector<uint64_t>& bbs) {
    std::bitset<64> bb_b(bb);
    for (int i = 0; i < 64; ++i) {
        if (bb_b[i] == 1) {
            bbs.emplace_back((uint64_t)1 << i);
        }
    }
}

void viz_bb(uint64_t bb){
    std::bitset<64>a(bb);
    for (int i = 7; i >= 0; i--){std::cout << a[i*8 + 0] <<"  "<< a[i*8 + 1]<<"  " << a[i*8 + 2] <<"  "<< a[i*8 + 3]<<"  " << a[i*8 + 4] <<"  "<< a[i*8 + 5] <<"  "<< a[i*8 + 6] <<"  "<< a[i*8 + 7]<<"  "<< std::endl;}
    std::cout<<"------------------------"<<std::endl;
}

void generate_bit(){
    char grid[8][8] = {
            //    r    n    b    q    k    b    n    r
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '} ,
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '} ,
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '} ,
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '} ,
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};
    unsigned int counter = -1;uint64_t bitboard = 0u;
    for(int i = 7; i >= 0; i--) {for(int j = 0; j <= 7; j++) {counter++;
            if(grid[i][j] == 'x' or grid[i][j] == 'X') {bitboard += (uint64_t)pow(2u, counter);}}}
    std::cout<<bitboard<<"u;"<<std::endl;
}

// todo: potential to make more efficient
// function that reverses the bits
uint64_t rev(uint64_t n)
{
    uint64_t r = 0;
    for(int i = 0; i < 64; i++)
    {
        r = r << 1 | (n & 1);
        n >>= 1;
    }
    // std::bitset<size> bits(r);
    //std::cout << "Reverse " << bits << std::endl;
    return r;
}

//todo: can optimize part of this calculation by simply making a lookup table, instead of calculating
std::string ind_x(int bit, int adder){return std::to_string(((bit - bit % 8) / 8) + adder);}
std::string ind_y(int bit, int adder){return std::to_string((bit % 8) + adder);}
std::string b_2_ind(int bit, int x_adder, int y_adder){
    return ind_x(bit, x_adder) + ind_y(bit, y_adder) + ">" + ind_x(bit, 0) + ind_y(bit, 0);
}