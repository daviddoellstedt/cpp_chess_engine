//
// Created by David Doellstedt on 5/31/20.
//

#include "Helper_functions.h"
#include <bitset>
#include <iostream>
#include <cmath>
#include <stdint.h>

void viz_bb(uint64_t bb){
    std::bitset<64>a(bb);
    for (int i = 7; i >= 0; i--){std::cout << a[i*8 + 0] <<"  "<< a[i*8 + 1]<<"  " << a[i*8 + 2] <<"  "<< a[i*8 + 3]<<"  " << a[i*8 + 4] <<"  "<< a[i*8 + 5] <<"  "<< a[i*8 + 6] <<"  "<< a[i*8 + 7]<<"  "<< std::endl;}
    std::cout<<"------------------------"<<std::endl;
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
