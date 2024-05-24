//
// Created by David Doellstedt on 5/12/20.
//
#include <iostream>
#include <string>
#include <bitset>
#include <cmath>
#include <vector>
#include <map>
#include <cstdlib>
#include <ctime>
#include <chrono>  // for high_resolution_clock
#include "Players.h"
#include "Helper_functions.h"
#include <stdint.h>
#include "bit_fns.h"


struct AI_return {
    std::string move;
    double value;
};


// Bit masks for ranks 1 - 8.
uint64_t rank_masks [8] = {0xFF, 0xFF00, 0xFF0000, 0xFF000000, 0xFF00000000, 0xFF0000000000, 0xFF000000000000, 0xFF00000000000000};

// Bit masls for files A - G.
uint64_t file_masks [8] = {0x101010101010101, 0x202020202020202, 0x404040404040404,  \
                                    0x808080808080808, 0x1010101010101010, 0x2020202020202020, \
                                    0x4040404040404040, 0x8080808080808080};  //left to right (file A-G)

//Bit masks for right/down diagnols (start at A1 end at H8).
uint64_t diag_dn_r_masks [15] = {1u, 258u, 66052u, 16909320u, 4328785936u, 1108169199648u, 283691315109952u, \
                                    72624976668147840u, 145249953336295424u, 290499906672525312u, 580999813328273408u, \
                                    1161999622361579520u, 2323998145211531264u, 4647714815446351872u, \
                                    9223372036854775808u};

//Bit masks for left/up diagnols (start at A8 end at H1).
uint64_t diag_up_r_masks [15] = {72057594037927936u, 144396663052566528u, 288794425616760832u, 577588855528488960u, \
                                    1155177711073755136u, 2310355422147575808u, 4620710844295151872u, \
                                    9241421688590303745u, 36099303471055874u, 141012904183812u, 550831656968u, \
                                    2151686160u, 8405024u, 32832u, 128u}; 

uint8_t loc_masks[64][4] = {{0, 0, 0, 7}, {0, 1, 1, 8}, {0, 2, 2, 9}, {0, 3, 3, 10}, {0, 4, 4, 11}, {0, 5, 5, 12}, {0, 6, 6, 13}, {0, 7, 7, 14},
                        {1, 0, 1, 6}, {1, 1, 2, 7}, {1, 2, 3, 8}, {1, 3, 4, 9},  {1, 4, 5, 10}, {1, 5, 6, 11}, {1, 6, 7, 12}, {1, 7, 8, 13},
                        {2, 0, 2, 5}, {2, 1, 3, 6}, {2, 2, 4, 7}, {2, 3, 5, 8},  {2, 4, 6, 9},  {2, 5, 7, 10}, {2, 6, 8, 11}, {2, 7, 9, 12},
                        {3, 0, 3, 4}, {3, 1, 4, 5}, {3, 2, 5, 6}, {3, 3, 6, 7},  {3, 4, 7, 8},  {3, 5, 8, 9},  {3, 6, 9, 10}, {3, 7, 10, 11},
                        {4, 0, 4, 3}, {4, 1, 5, 4}, {4, 2, 6, 5}, {4, 3, 7, 6},  {4, 4, 8, 7},  {4, 5, 9, 8},  {4, 6, 10, 9}, {4, 7, 11, 10},
                        {5, 0, 5, 2}, {5, 1, 6, 3}, {5, 2, 7, 4}, {5, 3, 8, 5},  {5, 4, 9, 6},  {5, 5, 10, 7}, {5, 6, 11, 8}, {5, 7, 12, 9},
                        {6, 0, 6, 1}, {6, 1, 7, 2}, {6, 2, 8, 3}, {6, 3, 9, 4},  {6, 4, 10, 5}, {6, 5, 11, 6}, {6, 6, 12, 7}, {6, 7, 13, 8},
                        {7, 0, 7, 0}, {7, 1, 8, 1}, {7, 2, 9, 2}, {7, 3, 10, 3}, {7, 4, 11, 4}, {7, 5, 12, 5}, {7, 6, 13, 6}, {7, 7, 14, 7}}; //mask index table, rank, file, diag down and right, diag up and right

uint64_t FILE_A = 72340172838076673u, FILE_H = 9259542123273814144u, FILE_AB = 217020518514230019u, FILE_GH = 13889313184910721216u;
uint64_t KNIGHT_MOVES = 345879119952u, KING_MOVES = 14721248u;
uint64_t  RANK_3 = 16711680u, RANK_4 = 4278190080u, RANK_5 = 1095216660480u, RANK_6 = 280375465082880u, RANK_8 = 18374686479671623680u,  FILLED = 18446744073709551615u;
uint64_t RANK_1 = 255u;
/** Printing the board to the command line.
*
* arguments: the 12 bitboards for the all the pieces
*/
void print_board(const GameState gamestate){
    char grid[8][8] = {
        //  | 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 7
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 6
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 5
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 4
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 3
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 2
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 1
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};// 0

    for(int8_t row = 7; row >= 0; row--){
        for(uint8_t col = 0; col < 8; col++){
            uint8_t bit = ((7 - row) * 8) + col;
            if (gamestate.white.pawn & (1ULL << bit)){
                grid[row][col] = 'P';
            }
            else if (gamestate.white.rook & (1ULL << bit)){
                grid[row][col] = 'R';
            }
            else if (gamestate.white.knight & (1ULL << bit)){
                grid[row][col] = 'N';
            }
            else if (gamestate.white.bishop & (1ULL << bit)){
                grid[row][col] = 'B';
            }
            else if (gamestate.white.queen & (1ULL << bit)){
                grid[row][col] = 'Q';
            }
            else if (gamestate.white.king & (1ULL << bit)){
                grid[row][col] = 'K';
            }
            else if (gamestate.black.pawn & (1ULL << bit)){
                grid[row][col] = 'p';
            }
            else if (gamestate.black.rook & (1ULL << bit)){
                grid[row][col] = 'r';
            }
            else if (gamestate.black.knight & (1ULL << bit)){
                grid[row][col] = 'n';
            }
            else if (gamestate.black.bishop & (1ULL << bit)){
                grid[row][col] = 'b';
            }
            else if (gamestate.black.queen & (1ULL << bit)){
                grid[row][col] = 'q';
            }
            else if (gamestate.black.king & (1ULL << bit)){
                grid[row][col] = 'k';
            }
        }
    }

    std::cout << "    |-----|-----|-----|-----|-----|-----|-----|-----|" << std::endl;
    std::cout<<"    |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |"<<std::endl;
    std::string line;
    for(int i = 0; i <= 7; i++) {
        std::cout << "|---|-----|-----|-----|-----|-----|-----|-----|-----|" << std::endl;
        line = "| " + std::to_string(i) + " |  ";
        for(int j = 0; j < 8; j++) {
            line.push_back(grid[i][j]);
            if(j != 7){line+= "  |  ";}
        }
        std::cout<<line + "  |"<<std::endl;
    }
    std::cout << "|---|-----|-----|-----|-----|-----|-----|-----|-----|" << std::endl;
}

/** Helper function used to convert a grid of pieces into the 12 unique bitboards. This should only be executed once,
* at the start of the game.
*
* @param g: the char grid of pieces
* @params 12 unique bitboards (by reference, so they can be modified)
*/
//todo: get rid of this eventually. Should be populating the bitboards directly from the FEN. Need to modify read FEN function also
void grid_to_bbs(char g[8][8], uint64_t& BR, uint64_t& BN, uint64_t& BB, \
                uint64_t& BQ, uint64_t& BK, uint64_t& BP, uint64_t& WR, \
                uint64_t& WN, uint64_t& WB, uint64_t& WQ, uint64_t& WK, \
                uint64_t& WP){
    int counter = -1;

    for(int i = 7; i >= 0; i--) {
        for(int j = 0; j < 8; j++) {
            counter++;

            if(g[i][j] == 'r') {BR += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'n') {BN += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'b') {BB += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'q') {BQ += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'k') {BK += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'p') {BP += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'R') {WR += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'N') {WN += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'B') {WB += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'Q') {WQ += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'K') {WK += (uint64_t)pow(2, counter);}
            else if (g[i][j] == 'P') {WP += (uint64_t)pow(2, counter);}
            else if (g[i][j] == ' ') {}  //do nothing
        }
    }
}

/** Function that can generate the possible moves a slider piece can make in the horizontal direction
*
* @param piece: bitboard representing a horizontal sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return horiz_moves: bitboard of horizontal moves
*/
uint64_t h_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED){
    uint64_t horiz_moves = (((OCCUPIED) - 2 * piece) ^ rev((rev(OCCUPIED) - 2 * rev(piece)))) & rank_masks[loc_masks[sl_bit][0]];
    return horiz_moves;
}

/** Function that can generate the possible moves a slider piece can make in the vertical direction
*
* @param piece: bitboard representing a vertical sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return vert_moves: bitboard of vertical moves
*/
uint64_t v_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED){
    uint64_t vert_moves = (((OCCUPIED & file_masks[loc_masks[sl_bit][1]]) - 2 * piece) ^ rev((rev(OCCUPIED & file_masks[loc_masks[sl_bit][1]]) - 2 * rev(piece)))) & file_masks[loc_masks[sl_bit][1]];
    return vert_moves;
}

/** Function that compiles the horizontal and vertical moves bitboards and handles a case where we check for unsafe moves for the king.
*
* @param piece: bitboard representing a horizontal/vertical sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @param unsafe_calc: flag used to see if we need to remove the enemy king from the occupied spaces (see inline comment for more details)
* @param K: bitboard representing king location. (see inline comment in the function for more details)
* @return bitboard of horizontal and vertical moves
*/
uint64_t h_v_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED, bool unsafe_calc, uint64_t K){

    // this line is used in the case where we need to generate zones for the king that are unsafe. If the king is in the
    // attack zone of a horizontal/vertical slider, we want to remove the king from the calculation. Because a move of the king, that
    // still lies in the path of attack (further away from the slider) is still an "unsafe" move.
    if (unsafe_calc){OCCUPIED &= ~K;}

    //todo: room for optimization? Less new variables?
    return h_moves(piece, sl_bit, OCCUPIED) | v_moves(piece, sl_bit, OCCUPIED);
}

/** Function that can generate the possible moves a slider piece can make in the (down, right) and (up, left) diagonol
*
* @param piece: bitboard representing a diagonal sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return ddr_moves: bitboard of (down, right) and (up, left) moves
*/
uint64_t ddr_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED){
    uint64_t ddr_moves = (((OCCUPIED & diag_dn_r_masks[loc_masks[sl_bit][2]]) - 2 * piece) ^ rev((rev(OCCUPIED & diag_dn_r_masks[loc_masks[sl_bit][2]]) - 2 * rev(piece)))) & diag_dn_r_masks[loc_masks[sl_bit][2]];
    return ddr_moves;
}

/** Function that can generate the possible moves a slider piece can make in the (up, right) and (down, left) diagonol
*
* @param piece: bitboard representing a diagonal sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return dur_moves: bitboard of (up, right) and (down, left) moves
*/
uint64_t dur_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED){
    uint64_t dur_moves = (((OCCUPIED & diag_up_r_masks[loc_masks[sl_bit][3]]) - 2 * piece) ^ rev((rev(OCCUPIED & diag_up_r_masks[loc_masks[sl_bit][3]]) - 2 * rev(piece)))) & diag_up_r_masks[loc_masks[sl_bit][3]];
    return dur_moves;
}

/** Function that compiles the diagonal moves bitboards and handles a case where we check for unsafe moves for the king.
*
* @param piece: bitboard representing a diagonal sliding piece
* @param sl_bit: the position of the set bit from 'piece' (log_2(piece))
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @param unsafe_calc: flag used to see if we need to remove the enemy king from the occupied spaces (see inline comment for more details)
* @param K: bitboard representing king location. (see inline comment in the function for more details)
* @return bitboard of all diagonal moves
*/
uint64_t diag_moves(uint64_t piece, int sl_bit, uint64_t OCCUPIED, bool unsafe_calc, uint64_t K){

    // this line is used in the case where we need to generate zones for the king that are unsafe. If the king is in the
    // attack zone of a diagonal slider, we want to remove the king from the calculation. Because a move of the king, that
    // still lies in the path of attack (further away from the slider) is still an "unsafe" move.
    if (unsafe_calc){OCCUPIED &= ~K;}
    //todo: room for optimization? Less new variables?
    return ddr_moves(piece, sl_bit, OCCUPIED) | dur_moves(piece, sl_bit, OCCUPIED);
}


/** Function that returns a bitboard mask of the straight line between two pieces. Inputs are guaranteed to be colinear
* at a diagonal or orthogonal perspective. More or less a lookup table.
*
* @param p1: first piece
* @param p2: second piece
* @return bitboard mask of rank/file/diagonal connection between the two pieces
*/
uint64_t get_mask(uint64_t p1, uint64_t p2){
    int k_bit = (int)log2(p2), p_bit = (int)log2(p1);

    int k_x = ((k_bit - k_bit % 8) / 8), p_x = ((p_bit - p_bit % 8) / 8);
    int k_y = (k_bit % 8), p_y = (p_bit % 8);

    if (k_x - p_x == 0){ //return horizontal mask
        return rank_masks[loc_masks[k_bit][0]];
    }
    else if (k_y - p_y == 0){  //return vertical mask
        return file_masks[loc_masks[k_bit][1]];
    }
    else if (((p_x - k_x) > 0 and (p_y - k_y) < 0) or ((p_x - k_x) < 0 and (p_y - k_y) > 0)){  //return ddr_mask
        return diag_dn_r_masks[loc_masks[k_bit][2]];
    }
    else if (((p_x - k_x) < 0 and (p_y - k_y) < 0) or ((p_x - k_x) > 0 and (p_y - k_y) > 0)) {  //return dur_mask
        return diag_up_r_masks[loc_masks[k_bit][3]];
    }
    else{
        std::cout<<"ERROR in get_pinned_mask"<<std::endl;
        return 0u;
    }
}

/** Function that returns a bitboard of pieces that are pinned. Pieces that if you were to move them, would place your
* own King in check (an illegal move).
*
* @params Piece bitboards (note that the E preceding a Piece letter denotes the enemies piece, ex: EB = enemy bishop)
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @return PINNED: bitboard of all pinned pieces for a color
*/
uint64_t get_pinned_pieces(uint64_t K, uint64_t P, uint64_t EQ, \
    uint64_t EB, uint64_t ER, uint64_t OCCUPIED, uint64_t E_P, \
    uint64_t& E_P_special){

    uint64_t PINNED = 0u, NEW_PIN, K_slider, H_moves;
    std::vector<uint64_t> h_v_slider_bbs, diag_slider_bbs; //individual bitboards for each piece
    ind_bbs(EQ | ER, h_v_slider_bbs);    //generate list of indivudal Queens and rooks
    ind_bbs(EQ | EB, diag_slider_bbs);    //generate list of indivudal Bishops and Queens

    //for the 4 directions (4 iterations)
    // 1. generate sliding moves from the kings position (include "capture" of the kings pieces
    // 2. generate sliding moves from opponents pieces
    // 3. take the AND of 1 and 2. This is a location of a pinned piece, if any. OR this result with pinned piece bb
    int k_bit = (int)log2(K);

    for (int i = 0; i <4; i++){
        if(i == 0){ // horizontal check
            K_slider = h_moves(K, k_bit, OCCUPIED);

            for (auto h: h_v_slider_bbs) {

                H_moves = h_moves(h, (int)log2(h), OCCUPIED);
                NEW_PIN = K_slider & H_moves;
                //todo: fix bug where the attacker moves are not detecting apinned pawn in the case of an en passant opportunity. Need to add a special check for E_P

                //todo add if logic for player turn

                if((((E_P << 8) & K_slider) != 0 and ((E_P << 7) & ~FILE_H & P & H_moves)) or (((E_P << 8) & H_moves) != 0 and ((E_P << 7) & ~FILE_H & P & K_slider))) {
                    NEW_PIN |= (E_P << 7);
                   // E_P_special = file_masks[loc_masks[(int)log2((E_P << 7))][1]];
                 //  E_P_special = get_mask((E_P << 7), E_P) | get_mask((E_P << 9), E_P);
                    E_P_special = (file_masks[loc_masks[(int)log2((E_P << 7))][1]] | diag_dn_r_masks[loc_masks[(int)log2((E_P << 7))][2]] | diag_up_r_masks[loc_masks[(int)log2((E_P << 7))][3]]) & ~get_mask((E_P << 7), E_P);

                    //  viz_bb(E_P_special);

                }

                if((((E_P << 8) & K_slider) != 0 and ((E_P << 9) & ~FILE_A & P & H_moves)) or (((E_P << 8) & H_moves) != 0 and ((E_P << 9) & ~FILE_A & P & K_slider))){
                    NEW_PIN |= (E_P << 9);
                    E_P_special = (file_masks[loc_masks[(int)log2((E_P << 9))][1]] | diag_dn_r_masks[loc_masks[(int)log2((E_P << 9))][2]] | diag_up_r_masks[loc_masks[(int)log2((E_P << 9))][3]]) & ~get_mask((E_P << 9), E_P);
                }


                //for white
                if((((E_P >> 8) & K_slider) != 0 and ((E_P >> 9) & ~FILE_H & P & H_moves)) or (((E_P >> 8) & H_moves) != 0 and ((E_P >> 9) & ~FILE_H & P & K_slider))) {
                    NEW_PIN |= (E_P >> 9);
                    //E_P_special = file_masks[loc_masks[(int)log2((E_P >> 9))][1]];
                   // E_P_special = get_mask((E_P << 7), E_P) | get_mask((E_P << 9), E_P);
                    E_P_special = (file_masks[loc_masks[(int)log2((E_P >> 9))][1]] | diag_dn_r_masks[loc_masks[(int)log2((E_P >> 9))][2]] | diag_up_r_masks[loc_masks[(int)log2((E_P >> 9))][3]]) & ~get_mask((E_P >> 9), E_P);


                }

                if((((E_P >> 8) & K_slider) != 0 and ((E_P >> 7) & ~FILE_A & P & H_moves)) or (((E_P >> 8) & H_moves) != 0 and ((E_P >> 7) & ~FILE_A & P & K_slider))){
                    NEW_PIN |= (E_P >> 7);
                   // E_P_special = file_masks[loc_masks[(int)log2((E_P >> 7))][1]];
                  //  E_P_special = get_mask((E_P << 7), E_P) | get_mask((E_P << 9), E_P);
                    E_P_special = (file_masks[loc_masks[(int)log2((E_P >> 7))][1]] | diag_dn_r_masks[loc_masks[(int)log2((E_P >> 7))][2]] | diag_up_r_masks[loc_masks[(int)log2((E_P >> 7))][3]]) & ~get_mask((E_P >> 7), E_P);


                }

                PINNED |= NEW_PIN;
            }
        }
        else if (i == 1){ //vertical check
            K_slider = v_moves(K, k_bit, OCCUPIED);
            for (auto v: h_v_slider_bbs) {
                NEW_PIN = K_slider & v_moves(v, (int)log2(v), OCCUPIED);
                PINNED |= NEW_PIN;
            }
        }
        else if (i == 2) { //ddr check
            K_slider = ddr_moves(K, k_bit, OCCUPIED);
            for (auto ddr: diag_slider_bbs) {
                NEW_PIN = K_slider & ddr_moves(ddr, (int)log2(ddr), OCCUPIED);
                PINNED |= NEW_PIN;
            }
        }
        else{   //dur check
            K_slider = dur_moves(K, k_bit, OCCUPIED);
            for (auto dur: diag_slider_bbs) {
                NEW_PIN = K_slider & dur_moves(dur, (int)log2(dur), OCCUPIED);
                PINNED |= NEW_PIN;
            }
        }
    }
    return PINNED;
}

/** Function that adds Rook moves to the move list
*
* @params Piece bitboards (Rook, King)
* @param PIECES: bitboard representing occupied spaces by the input player
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @param PINNED: bitboard of all pinned pieces for a color
* @param checker_zone: bitboard of check areas for the current king (enemy attacker piece(s) included).
* @param wb_moves: list of all possible moves for the inpout player. output will be appended to this variable.
*/
void get_rook_moves(uint64_t R, uint64_t K, uint64_t PIECES, \
    uint64_t OCCUPIED, uint64_t PINNED, uint64_t checker_zone, std::vector<std::string>& wb_moves){

    if(R != 0u) {
        if (checker_zone == 0){checker_zone = FILLED;}
        std::vector<uint64_t> r_bbs; //individual bitboards for each piece
        ind_bbs(R, r_bbs);    //generate list of indivudal bitboards
        uint64_t mask;

        for (auto bb: r_bbs) {

            // get moves
            int sl_bit = (int)log2(bb);

            mask = FILLED;
            if ((bb & PINNED) > 0u){mask = get_mask(bb, K);}
            //todo: make this a lookup table for improved performance
            std::bitset<64>moves(h_v_moves(bb, sl_bit, OCCUPIED, false, 0u) & ~PIECES & mask & checker_zone);
            // loop through moves and append to list, if there are any
            if (moves != 0) {
            //    viz_bb(h_v_moves(bb, sl_bit, OCCUPIED) & ~WHITE_PIECES);
             //   std::cout<<"----"<<std::endl;
                std::string ind_i = ind_x(sl_bit, 0) + ind_y(sl_bit, 0) + ">";
                for (int i = 0; i < 64; i++) {
                    if (moves[i] == 1) {
                        std::string ind_f = ind_x(i, 0) + ind_y(i, 0);
                        wb_moves.emplace_back(ind_i + ind_f);
                    }
                }
            }
        }
    }
}

/** Function that adds Bishop moves to the move list
*
* @params Piece bitboards (Bishop, King)
* @param PIECES: bitboard representing occupied spaces by the input player
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @param PINNED: bitboard of all pinned pieces for a color
* @param checker_zone: bitboard of check areas for the current king (enemy attacker piece(s) included).
* @param wb_moves: list of all possible moves for the inpout player. output will be appended to this variable.
*/
void get_bishop_moves(uint64_t B, uint64_t K, uint64_t PIECES, uint64_t OCCUPIED, uint64_t PINNED, uint64_t checker_zone, std::vector<std::string>& wb_moves){
    if(B != 0u) {
        if (checker_zone == 0){checker_zone = FILLED;}
        std::vector<uint64_t> bishop_bbs; //individual bitboards for each piece
        ind_bbs(B, bishop_bbs);    //generate list of indivudal bitboards
        uint64_t mask;

        for (auto bb: bishop_bbs) {
            // get moves
            int sl_bit = (int)log2(bb);

            mask = FILLED;
            if ((bb & PINNED) > 0u){mask = get_mask(bb, K);}
            //todo: make this a lookup table for improved performance
            std::bitset<64>moves(diag_moves(bb, sl_bit, OCCUPIED,false, 0u) & ~PIECES & mask & checker_zone);
            // loop through moves and append to list, if there are any
            if (moves != 0) {
                std::string ind_i = ind_x(sl_bit, 0) + ind_y(sl_bit, 0) + ">";
                //todo: can maybe optimize by not searching the entire range
                for (int i = 0; i < 64; i++) {
                    if (moves[i] == 1) {
                        std::string ind_f = ind_x(i, 0) + ind_y(i, 0);
                        wb_moves.emplace_back(ind_i + ind_f);
                    }
                }
            }
        }
    }
}

/** Function that adds Queen moves to the move list
*
* @params Piece bitboards (Queen, King)
* @param PIECES: bitboard representing occupied spaces by the input player
* @param OCCUPIED: bitboard representing all occupied spaces on the board
* @param PINNED: bitboard of all pinned pieces for a color
* @param checker_zone: bitboard of check areas for the current king (enemy attacker piece(s) included).
* @param wb_moves: list of all possible moves for the inpout player. output will be appended to this variable.
*/
void get_queen_moves(uint64_t Q, uint64_t K, uint64_t PIECES, uint64_t OCCUPIED, uint64_t PINNED, uint64_t checker_zone, std::vector<std::string>& wb_moves) {
    if (Q != 0u) {
        if (checker_zone == 0){checker_zone = FILLED;}
        std::vector<uint64_t> q_bbs; //individual bitboards for each piece
        ind_bbs(Q, q_bbs);    //generate list of indivudal bitboards
        uint64_t mask;

        for (auto bb: q_bbs) {
            // get moves
            int sl_bit = (int) log2(bb);

            mask = FILLED;
            if ((bb & PINNED) > 0u){mask = get_mask(bb, K);}
            //todo: make this a lookup table for improved performance
            std::bitset<64> moves((h_v_moves(bb, sl_bit, OCCUPIED, false, 0u) | diag_moves(bb, sl_bit, OCCUPIED, false, 0u)) & ~PIECES & mask & checker_zone);
            // loop through moves and append to list, if there are any
            if (moves != 0) {
                std::string ind_i = ind_x(sl_bit, 0) + ind_y(sl_bit, 0) + ">";
                for (int i = 0; i < 64; i++) {
                    if (moves[i] == 1) {
                        std::string ind_f = ind_x(i, 0) + ind_y(i, 0);
                        wb_moves.emplace_back(ind_i + ind_f);
                    }
                }
            }
        }
    }
}

/** Function that adds Knight moves to the move list
*
* @params Piece bitboards (Knight, King)
* @param PIECES: bitboard representing occupied spaces by the input player
* @param PINNED: bitboard of all pinned pieces for a color
* @param checker_zone: bitboard of check areas for the current king (enemy attacker piece(s) included).
* @param wb_moves: list of all possible moves for the inpout player. output will be appended to this variable.
*/
void get_knight_moves(uint64_t N, uint64_t K, uint64_t PIECES, uint64_t PINNED, uint64_t checker_zone, std::vector<std::string>& wb_moves) {
    if (N != 0u) {
        if (checker_zone == 0){checker_zone = FILLED;}
        std::vector<uint64_t> r_bbs; //individual bitboards for each piece
        ind_bbs(N, r_bbs);    //generate list of indivudal bitboards
        //todo: is it really efficient to redefine these everytime? maybe can optimize where this is defined
        //assuming knight is at bit 21 or F3 or (x3, y5)
        uint64_t pos_moves;

        for (auto bb: r_bbs) {
            // get moves\
            //todo: make this a lookup table for improved performance
            int kn_bit = (int) log2(bb);

            if ((bb & PINNED) == 0u) {   //only check for moves if it's not pinned. pinned knights cannot move.

                if (kn_bit > 21) {
                    pos_moves = KNIGHT_MOVES << (kn_bit - 21);
                } else {
                    pos_moves = KNIGHT_MOVES >> (21 - kn_bit);
                }
                if (kn_bit % 8 > 3) {
                    pos_moves &= ~FILE_AB;
                } else {
                    pos_moves &= ~FILE_GH;
                }
                pos_moves &= ~PIECES & checker_zone;

                //viz_bb(pos_moves);

                // todo: find a more efficient way to loop through the board (by being smart about it)
                std::bitset<64> moves(pos_moves);
                // loop through moves and append to list, if there are any
                if (moves != 0) {
                    std::string ind_i = ind_x(kn_bit, 0) + ind_y(kn_bit, 0) + ">";
                    for (int i = 0; i < 64; i++) {
                        if (moves[i] == 1) {
                            std::string ind_f = ind_x(i, 0) + ind_y(i, 0);
                            wb_moves.emplace_back(ind_i + ind_f);
                        }
                    }
                }
                pos_moves = 0u;
            }
        }
    }
}

/** Function that adds King moves to the move list
*
* @params Piece bitboards (King)
* @param PIECES: bitboard representing occupied spaces by the input player
* @param DZ: bitboard representing the current 'Danger Zone' for the King, which would put him in check if he moved there (illegal move)
* @param wb_moves: list of all possible moves for the inpout player. output will be appended to this variable.
*/
void get_king_moves(uint64_t K, uint64_t PIECES, uint64_t DZ, std::vector<std::string>& wb_moves) {
        //todo: is it really efficient to redefine these everytime? maybe can optimize where this is defined
         //assuming knight is at bit 21 or F3 or (x3, y5)
        uint64_t pos_moves;

        // get moves
        //todo: make this a lookup table for improved performance
        int k_bit = (int) log2(K);

        if (k_bit > 14) {
            pos_moves = KING_MOVES << (k_bit - 14);
        }
        else{
            pos_moves = KING_MOVES >> (14 - k_bit);
        }
        //todo: potential to make the FILEs more efficient
        if (k_bit % 8 > 3){
            pos_moves &= ~FILE_A;
        }
        else{
            pos_moves &= ~FILE_H;
        }
        pos_moves &= ~PIECES & ~DZ;

        // todo: find a more efficient way to loop through the board (by being smart about it)
        std::bitset<64>moves(pos_moves);
        // loop through moves and append to list, if there are any
        if (moves != 0) {
            std::string ind_i = ind_x(k_bit, 0) + ind_y(k_bit, 0) + ">";
            for (int i = 0; i < 64; i++) {
                if (moves[i] == 1) {
                    std::string ind_f = ind_x(i, 0) + ind_y(i, 0);
                    wb_moves.emplace_back(ind_i + ind_f);
                }
            }
        }
}

void get_X_pawn_moves(std::string X, uint64_t MASK, uint64_t P, uint64_t K, uint64_t E_P, uint64_t EMPTY, uint64_t OPP_PIECES, uint64_t checker_zone, std::vector<std::string>& moves){
    uint64_t P_FORWARD_1, P_FORWARD_2, P_ATTACK_L, P_ATTACK_R, P_PROMO_1, P_PROMO_L, P_PROMO_R;
    std::bitset<64> bits;

    if (X == "B") {
        P_FORWARD_1 = (P >> 8) & EMPTY & ~RANK_1 & MASK & checker_zone;
        P_FORWARD_2 = (P >> 16) & EMPTY & (EMPTY >> 8) & RANK_5 & MASK & checker_zone;
        P_ATTACK_L = (P >> 9) & OPP_PIECES & ~RANK_1 & ~FILE_H & MASK & checker_zone;
        P_ATTACK_R = (P >> 7) & OPP_PIECES & ~RANK_1 & ~FILE_A & MASK & checker_zone;
        P_PROMO_1 = (P >> 8) & EMPTY & RANK_1 & MASK & checker_zone;
        P_PROMO_L = (P >> 9) & OPP_PIECES & RANK_1 & ~FILE_H & MASK & checker_zone;
        P_PROMO_R = (P >> 7) & OPP_PIECES & RANK_1 & ~FILE_A & MASK & checker_zone;

      //  viz_bb(P_PROMO_L);

        //CHECK TO SEE IF WE CAN MOVE 1 SPACE FORWARD
        if (P_FORWARD_1 > 0u) {
            bits = P_FORWARD_1;    //check to see if you can move 1
            for (int i = 8; i < 48; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, 0));
                }
            }
        }

        if (P_FORWARD_2 > 0u) {
            bits = P_FORWARD_2;   //check to see if you can move 2
            for (int i = 32; i < 40; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 2, 0) + ">2");
                }
            }
        }

        if (P_ATTACK_L > 0u) {
            bits = P_ATTACK_L;     //check for attacks left
            for (int i = 8; i < 48; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, 1));
                }
            }
        }

        if (P_ATTACK_R > 0u) {
            bits = P_ATTACK_R;    //check for attacks right
            for (int i = 8; i < 48; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, -1));
                }
            }
        }

        //check for promotion straight
        if (P_PROMO_1 > 0u) {
            bits = P_PROMO_1;
            for (int i = 0; i < 8; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, 0) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, 1, 0) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, 1, 0) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, 1, 0) + ">" + "PN");
                }
            }
        }

        if (P_PROMO_L > 0u) {
            bits = P_PROMO_L;      //check for promotion left
            for (int i = 0; i < 8; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "PN");
                }
            }
        }

        if (P_PROMO_R > 0u) {
            bits = P_PROMO_R;      //check for promotion attack right
            for (int i = 0; i < 8; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "PN");
                }
            }
        }

        if(E_P != 0) {
            //todo: specialize this for white
            if (checker_zone != FILLED and ((E_P << 8) & checker_zone) != 0){
                checker_zone |= (file_masks[loc_masks[(int)log2(checker_zone)][1]] & RANK_3);
            }
          //  viz_bb(E_P << 8);
          //  viz_bb(checker_zone);
           // if((E_P & checker_zone) =! 0)
            uint64_t P_EP_L = (P >> 9) & E_P & ~FILE_H & MASK & checker_zone;
            uint64_t P_EP_R = (P >> 7) & E_P & ~FILE_A & MASK & checker_zone;

            if (P_EP_L > 0u) {
                //check for en passant left
                bits = P_EP_L;
         //       viz_bb(E_P);
           //     viz_bb(MASK);
             //   viz_bb(checker_zone);
                for (int i = 16; i < 24; i++) {
                    if (bits[i] == 1) {
                        moves.emplace_back(b_2_ind(i, 1, 1) + ">" + "EP");
                    }
                }
            }

            if (P_EP_R > 0u) {
                //check for en passant right
                bits = P_EP_R;
                for (int i = 16; i < 24; i++) {
                    if (bits[i] == 1) {
                        moves.emplace_back(b_2_ind(i, 1, -1) + ">" + "EP");
                    }
                }
            }
        }
    }
    else {    //case for W

        P_FORWARD_1 = (P << 8) & EMPTY & ~RANK_8 & MASK & checker_zone;
        P_FORWARD_2 = (P << 16) & EMPTY & (EMPTY << 8) & RANK_4 & MASK & checker_zone;
        P_ATTACK_L = (P << 7) & OPP_PIECES & ~RANK_8 & ~FILE_H & MASK & checker_zone;
        P_ATTACK_R = (P << 9) & OPP_PIECES & ~RANK_8 & ~FILE_A & MASK & checker_zone;
        P_PROMO_1 = (P << 8) & EMPTY & RANK_8 & MASK & checker_zone;
        P_PROMO_L = (P << 7) & OPP_PIECES & RANK_8 & ~FILE_H & MASK & checker_zone;
        P_PROMO_R = (P << 9) & OPP_PIECES & RANK_8 & ~FILE_A & MASK & checker_zone;

        if (P_FORWARD_1 > 0u) {

            bits = P_FORWARD_1;   // check to see if you can move 1
            for (int i = 16; i < 56; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, -1, 0));
                }
            }
        }

        if (P_FORWARD_2 > 0u) {
            bits = P_FORWARD_2;        // check to see if you can move 2
            for (int i = 24; i < 32; i++) {

                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, -2, 0)+ ">2");
                }
            }
        }

        if (P_ATTACK_L > 0u) {
            bits = P_ATTACK_L;         //check for attacks left
            for (int i = 16; i < 56; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, -1, 1));
                }
            }
        }

        if (P_ATTACK_R > 0u) {
            bits = P_ATTACK_R;         //check for attacks right
            for (int i = 16; i < 56; i++) {
                if (bits[i] == 1) {
                    moves.emplace_back(b_2_ind(i, -1, -1));
                }
            }
        }

        if (P_PROMO_1 > 0u) {
            bits = P_PROMO_1;          //check for promotion straight
            for (int i = 56; i < 64; i++) {
                if (bits[i] == 1) {
                    //todo: can i optimize by storing a ind string variable
                    moves.emplace_back(b_2_ind(i, -1, 0) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, -1, 0) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, -1, 0) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, -1, 0) + ">" + "PN");
                }
            }
        }

        if (P_PROMO_L > 0u) {
            bits = P_PROMO_L;          //check for promotion left
            for (int i = 56; i < 64; i++) {
                if (bits[i] == 1) {
                    //todo: can i optimize by storing a ind string variable
                    moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "PN");
                }
            }
        }

        if (P_PROMO_R > 0u) {
            bits = P_PROMO_R;          //check for promotion attack right
            for (int i = 56; i < 64; i++) {
                if (bits[i] == 1) {
                    //todo: can i optimize by storing a ind string variable
                    moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "PQ");
                    moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "PR");
                    moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "PB");
                    moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "PN");
                }
            }
        }

        if(E_P != 0) {
            if (checker_zone != FILLED and ((E_P >> 8) & checker_zone) != 0){
                checker_zone |= (file_masks[loc_masks[(int)log2(checker_zone)][1]] & RANK_6);
            }

            uint64_t P_EP_L = (P << 7) & E_P & ~FILE_H & MASK & checker_zone;
            uint64_t P_EP_R = (P << 9) & E_P & ~FILE_A & MASK & checker_zone;

            if (P_EP_L != 0u) {
                //check for en passant left
                bits = P_EP_L;
                for (int i = 40; i < 48; i++) {
                    if (bits[i] == 1) {

                        moves.emplace_back(b_2_ind(i, -1, 1) + ">" + "EP");

                    }
                }
            }

            if (P_EP_R != 0u) {
                //check for en passant right
                bits = P_EP_R;
                for (int i = 40; i < 48; i++) {
                    if (bits[i] == 1) {
                        moves.emplace_back(b_2_ind(i, -1, -1) + ">" + "EP");
                    }
                }
            }
        }

    }
}
void get_B_pawn_moves(uint64_t BP, uint64_t BK, uint64_t E_P, uint64_t EMPTY, uint64_t WHITE_PIECES, uint64_t PINNED, uint64_t checker_zone, uint64_t E_P_SPECIAL, std::vector<std::string>& b_moves) {

    uint64_t mask;
    uint64_t pinned_pawns = (BP & PINNED);
    if (checker_zone == 0){checker_zone = FILLED;}
    if (pinned_pawns > 0u){  //we have at least 1 pawn pinned
     //   viz_bb((BP & PINNED));
        BP &= ~PINNED;
        std::vector<uint64_t> pin_bbs; //individual bitboards for each piece
        ind_bbs(pinned_pawns, pin_bbs);
        for (auto bb: pin_bbs) {
            //std::cout<<bb<<std::endl;
            mask = (get_mask(bb, BK) | E_P_SPECIAL) ;
          //  viz_bb(bb);
            //viz_bb(mask);
          //  viz_bb(E_P_SPECIAL);
         //   viz_bb(E_P);
        // std::cout<<"pinned"<<std::endl;
            get_X_pawn_moves("B", mask, bb, BK, E_P, EMPTY, WHITE_PIECES, checker_zone, b_moves);


        }
    }

    if (BP > 0u){   // we have at least 1 non-pinned pawn
        mask = FILLED;
        get_X_pawn_moves("B", mask, BP, BK, E_P, EMPTY, WHITE_PIECES, checker_zone, b_moves);

    }
}
void get_W_pawn_moves(uint64_t WP, uint64_t WK, uint64_t E_P, uint64_t EMPTY, uint64_t BLACK_PIECES, uint64_t PINNED, uint64_t checker_zone, uint64_t E_P_SPECIAL, std::vector<std::string>& w_moves){

    uint64_t mask;
    uint64_t pinned_pawns = (WP & PINNED);
    if (checker_zone == 0){checker_zone = FILLED;}

    if (pinned_pawns > 0u){  //we have at least 1 pawn pinned
        WP &= ~PINNED;

        std::vector<uint64_t> pin_bbs; //individual bitboards for each piece
        ind_bbs(pinned_pawns, pin_bbs);
        for (auto bb: pin_bbs) {
            mask = get_mask(bb, WK) | E_P_SPECIAL;

        //    if(bb & )

            get_X_pawn_moves("W", mask, bb, WK, E_P, EMPTY, BLACK_PIECES, checker_zone, w_moves);
        }
    }

    if (WP > 0u){   // we have at least 1 non-pinned pawn
        mask = FILLED;
        get_X_pawn_moves("W", mask, WP, WK, E_P, EMPTY, BLACK_PIECES, checker_zone, w_moves);
    }
}

void get_K_castle(bool CK, uint64_t K, uint64_t EMPTY, uint64_t DZ, std::vector<std::string>& wb_moves){
    if(CK) {
        //todo: implement lookup table
        if(((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1)) != 0u){
            int k_bit = (int) log2(((K << 2) & EMPTY & (EMPTY << 1) & ~DZ & ~(DZ << 1)));
            wb_moves.emplace_back(b_2_ind(k_bit, 0, -2) + ">CK");
        }
    }
}
void get_Q_castle(bool QK, uint64_t K, uint64_t EMPTY, uint64_t DZ, std::vector<std::string>& wb_moves){

    if(QK) {
     //   viz_bb(DZ);
       // viz_bb(((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1));
        //todo: implement lookup table
        if((((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1)) != 0u){
            int k_bit = (int) log2((((K >> 2) & EMPTY) & (EMPTY >> 1) & (EMPTY << 1) & ~DZ & ~(DZ >> 1)));
            wb_moves.emplace_back(b_2_ind(k_bit, 0, 2) + ">CQ");
        }
    }
}

uint64_t unsafe_for_XK(std::string X, uint64_t P, uint64_t R, uint64_t N, \
                        uint64_t B, uint64_t Q, uint64_t K, uint64_t EK, uint64_t OCCUPIED) {

    uint64_t unsafe = 0u, D = B | Q, HV = R | Q;

    //pawn
    if (P != 0u) {
        if (X == "B") {
            unsafe = (P << 9) & ~FILE_A;  //capture right
            unsafe |= (P << 7) & ~FILE_H; //capture left
        }
        else {
            unsafe = (P >> 7) & ~FILE_A;  //capture right
            unsafe |= (P >> 9) & ~FILE_H; //capture left
        }
    }

    //knight
    uint64_t pos_moves;
    if (N != 0u) {
        std::vector<uint64_t> r_bbs; //individual bitboards for each piece
        ind_bbs(N, r_bbs);    //generate list of indivudal bitboards
        //todo: is it really efficient to redefine these everytime? maybe can optimize where this is defined
        //assuming knight is at bit 21 or F3 or (x3, y5)
        for (auto bb: r_bbs) {
            //todo: make this a lookup table for improved performance
            int kn_bit = (int) log2(bb);
            if (kn_bit > 21) { pos_moves = KNIGHT_MOVES << (kn_bit - 21); }
            else { pos_moves = KNIGHT_MOVES >> (21 - kn_bit); }
            if (kn_bit % 8 > 3) { pos_moves &= ~FILE_AB; }
            else { pos_moves &= ~FILE_GH; }
            //viz_bb(pos_n_moves);
            unsafe |= pos_moves;
        }



    }

    //diag pieces (Bishop, Queen)
    if (D != 0u) {
        std::vector<uint64_t> r_bbs; //individual bitboards for each piece
        ind_bbs(D, r_bbs);    //generate list of indivudal bitboards
        for (auto bb: r_bbs) {
            //todo: make a lookup table to avoid the log calc
            int sl_bit = (int) log2(bb);
            unsafe |= diag_moves(bb, sl_bit, OCCUPIED, true, EK);
        }
    }

    //hv pieces (Rook, Queen)
    if (HV != 0u) {
        //todo: should i keep redeclaring this variable? Orsimply wipe the last instance?
        std::vector<uint64_t> r_bbs; //individual bitboards for each piece
        ind_bbs(HV, r_bbs);    //generate list of indivudal bitboards
        for (auto bb: r_bbs) {
            //todo: make a lookup table to avoid the log calc
            int sl_bit = (int) log2(bb);
            unsafe |= h_v_moves(bb, sl_bit, OCCUPIED, true, EK);
        }

    }

    //king
    //todo: make this a lookup table for improved performance
    int k_bit = (int) log2(K);
    if (k_bit > 14) {pos_moves = KING_MOVES << (k_bit - 14);}
    else {pos_moves = KING_MOVES >> (14 - k_bit);}
    //todo: potential to make the FILEs more efficient
    if (k_bit % 8 > 3) {pos_moves &= ~FILE_A;}
    else {pos_moves &= ~FILE_H;}
    unsafe |= pos_moves;
    //end

    return unsafe;
}

void get_B_moves(GameState& gamestate, uint64_t E_P, bool& CM, bool& SM, std::vector<std::string>& b_moves){

    uint64_t BLACK_PIECES = gamestate.black.pawn | gamestate.black.rook | gamestate.black.knight | gamestate.black.bishop | gamestate.black.queen | gamestate.black.king;
    uint64_t WHITE_PIECES = gamestate.white.pawn | gamestate.white.rook | gamestate.white.knight | gamestate.white.bishop | gamestate.white.queen | gamestate.white.king; 
    uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    uint64_t DZ = unsafe_for_XK("B", gamestate.white.pawn, gamestate.white.rook, gamestate.white.knight, gamestate.white.bishop, \
                                gamestate.white.queen, gamestate.white.king, gamestate.black.king, OCCUPIED);
    b_moves.clear();
    uint64_t E_P_SPECIAL = 0u;

    //DZ is the danger zone. If the king is inside of it, its in check.
    int num_checkers = 0;
    uint64_t PINNED = get_pinned_pieces(gamestate.black.king, gamestate.black.pawn, gamestate.white.queen, gamestate.white.bishop, gamestate.white.rook, OCCUPIED, E_P, E_P_SPECIAL); //todo: need to put this to work. dont generate pinned moves if in check, skip that piece
    bool check = (DZ & gamestate.black.king) != 0u;
// ------------------
    uint64_t checkers = 0, new_checker, checker_zone = 0;    //checker zone is the area that the piece is attacking through (applies only to sliders). We have the potential to block the check by moving  apiece in the line of fire (pinning your own piece)

    // ------------------
    if(check) {  //currently in check
    // todo: generate checkers_bb, update_num_checkers. create method.
        uint64_t HV = gamestate.white.rook | gamestate.white.queen;
        int k_bit = (int)log2(gamestate.black.king);
        uint64_t K_moves;

        //check horizontal pieces
        K_moves = h_moves(gamestate.black.king, k_bit, OCCUPIED);
        new_checker = K_moves & HV;
        if (new_checker != 0u) {
            checkers |= new_checker;
            checker_zone |= h_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check vertical pieces
        K_moves = v_moves(gamestate.black.king, k_bit, OCCUPIED);
        new_checker = K_moves & HV;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= v_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        uint64_t D = gamestate.white.bishop | gamestate.white.queen;
        //check down and to the right pieces
        K_moves = ddr_moves(gamestate.black.king, k_bit, OCCUPIED);
        new_checker = K_moves & D;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= ddr_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check up and to the right pieces
        K_moves = dur_moves(gamestate.black.king, k_bit, OCCUPIED);
        new_checker = K_moves & D;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= dur_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check for knight attacks
        if (k_bit > 21) {K_moves = KNIGHT_MOVES << (k_bit - 21);}
        else {K_moves = KNIGHT_MOVES >> (21 - k_bit);}
        if (k_bit % 8 > 3) {
            K_moves &= ~FILE_AB;
        } else {
            K_moves &= ~FILE_GH;
        }
        new_checker = K_moves & gamestate.white.knight;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }


        //check for pawn right attack (from pawns perspective)
        K_moves = (gamestate.black.king >> 9) & ~FILE_H;
        //viz_bb(K_moves & WP);
        new_checker = K_moves & gamestate.white.pawn;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }

        //check for pawn left attack (from pawns perspective)
        K_moves = (gamestate.black.king >> 7) & ~FILE_A;
      //  viz_bb(K_moves & WP);
        new_checker = K_moves & gamestate.white.pawn;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }
        //  viz_bb(checkers);
        // viz_bb(checker_zone);

    }
    else{   //only search for castles if you aren't in check
        get_K_castle(gamestate.black.can_king_side_castle, gamestate.black.king, ~OCCUPIED, DZ, b_moves);
        get_Q_castle(gamestate.black.can_queen_side_castle, gamestate.black.king, ~OCCUPIED, DZ, b_moves);
    }

    checker_zone |= checkers;
   // viz_bb(checker_zone);
   // std::cout<<num_checkers<<std::endl;

// todo: pass check zones into the files

    if (num_checkers < 2 ) {

        get_B_pawn_moves(gamestate.black.pawn, gamestate.black.king, E_P, ~OCCUPIED, WHITE_PIECES, PINNED, checker_zone, E_P_SPECIAL, b_moves);
        get_rook_moves(gamestate.black.rook, gamestate.black.king, BLACK_PIECES, OCCUPIED, PINNED, checker_zone, b_moves);
        get_bishop_moves(gamestate.black.bishop, gamestate.black.king, BLACK_PIECES, OCCUPIED, PINNED, checker_zone, b_moves);
        get_queen_moves(gamestate.black.queen, gamestate.black.king, BLACK_PIECES, OCCUPIED, PINNED, checker_zone,b_moves);
        get_knight_moves(gamestate.black.knight, gamestate.black.king, BLACK_PIECES, PINNED, checker_zone, b_moves);

    }
    get_king_moves(gamestate.black.king, BLACK_PIECES, DZ, b_moves);

    if (b_moves.empty() and check){CM = true;}
    else if (b_moves.empty() and !check){SM = true;}
    else if ((gamestate.black.king | gamestate.white.king) == OCCUPIED){SM = true;}

   // return check;

}

void get_W_moves(const GameState& gamestate, uint64_t E_P, bool& CM, bool& SM, std::vector<std::string>& w_moves){

    uint64_t BLACK_PIECES = gamestate.black.pawn | gamestate.black.rook | gamestate.black.knight | gamestate.black.bishop | gamestate.black.queen | gamestate.black.king;
    uint64_t WHITE_PIECES = gamestate.white.pawn | gamestate.white.rook | gamestate.white.knight | gamestate.white.bishop | gamestate.white.queen | gamestate.white.king; 
    uint64_t OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    w_moves.clear();

    uint64_t E_P_SPECIAL = 0u;

    uint64_t DZ = unsafe_for_XK("W", gamestate.black.pawn, gamestate.black.rook, gamestate.black.knight, gamestate.black.bishop, \
                                gamestate.black.queen, gamestate.black.king, gamestate.white.king, OCCUPIED);
   // viz_bb(DZ);
    //DZ is the danger zone. If the king is inside of it, its in check.
    int num_checkers = 0;
    uint64_t PINNED = get_pinned_pieces(gamestate.white.king, gamestate.white.pawn, gamestate.black.queen, gamestate.black.bishop, \
                                        gamestate.black.rook, OCCUPIED, E_P, E_P_SPECIAL); //todo: need to put this to work. dont generate pinned moves if in check, skip that piece
    bool check = (DZ & gamestate.white.king) != 0u;
// ------------------
    uint64_t checkers = 0, new_checker, checker_zone = 0;    //checker zone is the area that the piece is attacking through (applies only to sliders). We have the potential to block the check by moving  apiece in the line of fire (pinning your own piece)

    // ------------------
    if(check) {  //currently in check
        // todo: generate checkers_bb, update_num_checkers. create method.
        uint64_t HV = gamestate.black.rook | gamestate.black.queen;
        int k_bit = (int)log2(gamestate.white.king);
        uint64_t K_moves;

        //check horizontal pieces
        K_moves = h_moves(gamestate.white.king, k_bit, OCCUPIED);
        new_checker = K_moves & HV;
        if (new_checker != 0u) {
            checkers |= new_checker;
            checker_zone |= h_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check vertical pieces
        K_moves = v_moves(gamestate.white.king, k_bit, OCCUPIED);
        new_checker = K_moves & HV;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= v_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        uint64_t D = gamestate.black.bishop | gamestate.black.queen;
        //check down and to the right pieces
        K_moves = ddr_moves(gamestate.white.king, k_bit, OCCUPIED);
        new_checker = K_moves & D;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= ddr_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check up and to the right pieces
        K_moves = dur_moves(gamestate.white.king, k_bit, OCCUPIED);
        new_checker = K_moves & D;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            checker_zone |= dur_moves(new_checker, (int)log2(new_checker), OCCUPIED) & K_moves;
            num_checkers++;
        }

        //check for knight attacks
        if (k_bit > 21) {
            K_moves = KNIGHT_MOVES << (k_bit - 21);}
        else {
            K_moves = KNIGHT_MOVES >> (21 - k_bit);}

        if (k_bit % 8 > 3) {
            K_moves &= ~FILE_AB;
        } else {
            K_moves &= ~FILE_GH;
        }

        new_checker = K_moves & gamestate.black.knight;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }

        //check for pawn right attack (from pawns perspective)
        K_moves = (gamestate.white.king << 7) & ~FILE_H;   //todo: verify
        //viz_bb(K_moves & WP);
        new_checker = K_moves & gamestate.black.pawn;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }
        //viz_bb(checkers);

        //check for pawn left attack (from pawns perspective)
        K_moves = (gamestate.white.king << 9) & ~FILE_A;  //todo: verify
        //  viz_bb(K_moves & WP);
        new_checker = K_moves & gamestate.black.pawn;
        if (new_checker != 0u and num_checkers != 2) {
            checkers |= new_checker;
            num_checkers++;
        }
        //  viz_bb(checkers);
        // viz_bb(checker_zone);

    }
    else{   //only search for castles if you aren't in check
        get_K_castle(gamestate.white.can_king_side_castle, gamestate.white.king, ~OCCUPIED, DZ, w_moves);
        get_Q_castle(gamestate.white.can_queen_side_castle, gamestate.white.king, ~OCCUPIED, DZ, w_moves);
    }

    checker_zone |= checkers;
// todo: pass check zones into the files

    if (num_checkers < 2 ) {
        get_W_pawn_moves(gamestate.white.pawn, gamestate.white.king, E_P, ~OCCUPIED, BLACK_PIECES, PINNED, checker_zone, E_P_SPECIAL, w_moves);
        get_rook_moves(gamestate.white.rook, gamestate.white.king, WHITE_PIECES, OCCUPIED, PINNED, checker_zone, w_moves);
        get_bishop_moves(gamestate.white.bishop, gamestate.white.king, WHITE_PIECES, OCCUPIED, PINNED, checker_zone, w_moves);
        get_queen_moves(gamestate.white.queen, gamestate.white.king, WHITE_PIECES, OCCUPIED, PINNED, checker_zone,w_moves);
        get_knight_moves(gamestate.white.knight, gamestate.white.king, WHITE_PIECES, PINNED, checker_zone, w_moves);

    }
    get_king_moves(gamestate.white.king, WHITE_PIECES, DZ, w_moves);

    if (w_moves.empty() and check){CM = true;}
    else if (w_moves.empty() and !check){SM = true;}
    else if ((gamestate.black.king | gamestate.white.king) == OCCUPIED){SM = true;}

  //  return check;
}

void apply_move(bool& white_move, std::string move, uint64_t& R, uint64_t& N, uint64_t& B, \
                uint64_t& Q, uint64_t& K, uint64_t& P, uint64_t& OR, \
                uint64_t& ON, uint64_t& OB, uint64_t& OQ, uint64_t& OK, \
                uint64_t& OP, uint64_t& E_P, bool& WCK, bool& WCQ, bool& BCK, bool& BCQ){

    uint64_t WHITE_PIECES, BLACK_PIECES;

    if (white_move) {
        WHITE_PIECES = (R | N | B | Q | K | P);
        BLACK_PIECES = (OR | ON | OB | OQ | OK | OP);
    }
    else{
        BLACK_PIECES = (R | N | B | Q | K | P);
        WHITE_PIECES = (OR | ON | OB | OQ | OK | OP);
    }

    int x1 = stoi(move.substr(0,1)), y1 = stoi(move.substr(1,1));
    int x2 = stoi(move.substr(3,1)), y2 = stoi(move.substr(4,1));

    uint64_t initial = pow(2,((x1 * 8) + (y1 % 8)));
    uint64_t final = pow(2,((x2 * 8) + (y2 % 8)));


    //-----discover what piece is being moved-----
//    uint64_t moving_piece;
//    if ((Q & initial) != 0){moving_piece = Q;}
//    else if ((B & initial) != 0){moving_piece = B;}
//    else if ((R & initial) != 0){moving_piece = R;}
//    else if ((N & initial) != 0){moving_piece = N;}
//    else if ((P & initial) != 0){moving_piece = P;}
//    else if ((K & initial) != 0){moving_piece = K;}

    //-----remove enemy pieces in the case of a capture-----
    uint64_t capture;

    if (white_move){
        capture = (BLACK_PIECES & ~OK) & final;} else{ capture = WHITE_PIECES & ~OK & final;}

    if (capture != 0) {
        if ((OP & final) != 0) {
            OP &= ~final;
             //cap_counter++;
        } else if ((OR & final) != 0) {

            if (white_move){

                if((OR & 9223372036854775808u & final) == 9223372036854775808u){BCK = false;
                }
                else if ((OR & 72057594037927936u & final) == 72057594037927936u){BCQ = false;
              //      std::cout<<"hi"<<std::endl;
              //
              }
            }
            else{
                if((OR & 128u & final) == 128u){WCK = false;}
                else if ((OR & 1u &final) == 1u){WCQ = false;}
            }

            OR &= ~final;
            //cap_counter++;
        } else if ((ON & final) != 0) {
            ON &= ~final;
          //  cap_counter++;
        } else if ((OB & final) != 0) {
            OB &= ~final;
         //   cap_counter++;
//            std::cout << "opponent bishops" << std::endl;
//            viz_bb(OB & final);
//            std::cout << "init" << std::endl;
//            viz_bb(initial);
//            std::cout << "final" << std::endl;
//
//            viz_bb(final);
        } else if ((OQ & final) != 0) {
            OQ &= ~final;
         //   cap_counter++;
            // ;
        }

    }
    else if (E_P == final and ((initial & P) != 0)){ //this means there was an en passant capture

        if (white_move){OP &= ~(final >> 8);}
        else{OP &= ~(final << 8);}
       // cap_counter++;

       // std::cout<<"HEY"<<std::endl;
       // viz_bb(final);
    }
   // -----this concludes removing enemy pieces from board-----


   // need to move piece to the final position and also remove the initial position
   if (move.length() == 5){
    //   std::cout<<"hello"<<std::endl;
       //viz_bb(P);
    //todo: search for which piece is moving
        if ((Q & initial) != 0){
            Q |= final;
            Q &= ~initial;
        }
        else if ((B & initial) != 0){
            B |= final;
            B &= ~initial;
        }
        else if ((R & initial) != 0){

            if (WCK and white_move and (R & 128u & initial) == 128u){WCK = false;}
            else if (WCQ and white_move and (R & 1u & initial) == 1u){WCQ = false;}
            else if (BCK and !white_move and (R & 9223372036854775808u & initial) == 9223372036854775808u){BCK = false;}
            else if (BCQ and !white_move and (R & 72057594037927936u & initial) == 72057594037927936u){BCQ = false;

            //std::cout<<"HEY"<<std::endl;
            }
            R |= final;
            R &= ~initial;
        }
        else if ((N & initial) != 0){
            N |= final;
            N &= ~initial;
        }
        else if ((P & initial) != 0){
         //   std::cout<<"hello2"<<std::endl;

            P |= final;
            P &= ~initial;
        }
        else if ((K & initial) != 0){
            if ((WCK or WCQ) and white_move){WCK = false; WCQ = false;}
            if ((BCK or BCQ) and !white_move){BCK = false; BCQ = false;}
          //  std::cout<<WCK<<std::endl;
            K = final;
        }

        E_P = 0u;
   }
   else{
       if (move.substr(6,1) == "C"){    //castling
           if(white_move){
               if (move.substr(7,1) == "K"){ //kingside castle for white

                   K = 64u; R -= 128u; R += 32u;
                   WCK = false; WCQ = false;
               }
               else{    //queenside castle for black
                   K = 4u; R -= 1u; R += 8u;
                   WCQ = false; WCK = false;
               }
           }
           else{

               if (move.substr(7,1) == "K"){ //kingside castle for black
                   K = 4611686018427387904u; R -= 9223372036854775808u; R += 2305843009213693952u;
                   BCK = false; BCQ = false;
               }
               else{    //queenside castle for black
                   K = 288230376151711744u; R -= 72057594037927936u; R += 576460752303423488u;
                   BCQ = false; BCK = false;
               }
           }
           E_P = 0u;
       }
       else if (move.substr(6,1) == "P"){   //promotion
           P &= ~initial;
           if(move.substr(7,1) == "Q"){Q |= final;}
           else if (move.substr(7,1) == "R"){R |= final;}
           else if (move.substr(7,1) == "N"){N |= final;}
           else if (move.substr(7,1) == "B"){B |= final;}
           E_P = 0u;
       }
       else if (move.substr(6,1) == "E"){    //en passant capture

           P |= final;
           P &= ~initial;
           E_P = 0u;
       }
       else { // pawn push 2

           P |= final;
           P &= ~initial;
           if (white_move){E_P = (final >> 8);}
           else{E_P = (final << 8);}


       }
   }

   white_move = !white_move;   //alternate the turn


}



void print_moves(bool white_move, std::vector<std::string> b_moves, std::vector<std::string> w_moves){

    if (white_move){
        std::cout<< "WHITE'S MOVE: "<<std::endl;
        for (int i = 0; i < w_moves.size(); i++) {
            std::cout << i + 1 << ": " + w_moves[i] << std::endl;
        }
      //  std::cout << "total moves: " << w_moves.size() << std::endl;
    }
    else{
        std::cout<< "BLACK'S MOVE: "<<std::endl;
        for (int i = 0; i < b_moves.size(); i++) {
            std::cout << i + 1 << ": " + b_moves[i] << std::endl;
        }
    }
}

bool aa = false;

void perft(uint32_t& nodes, uint32_t& cap_counter, GameState& gamestate, std::vector<std::string> moves, uint64_t& E_P, bool CM, bool SM, int depth, int orig_depth, std::string n){

            // Make a function.
            // BLACK_PIECES = gamestate.black.pawn | gamestate.black.rook | gamestate.black.knight | gamestate.black.bishop | gamestate.black.queen | gamestate.black.king; 
            // WHITE_PIECES = gamestate.white.pawn | gamestate.white.rook | gamestate.white.knight | gamestate.white.bishop | gamestate.white.queen | gamestate.white.king; 
            // OCCUPIED = BLACK_PIECES | WHITE_PIECES;
            bool check = false;

        uint64_t BR = gamestate.black.rook;
        uint64_t BN = gamestate.black.knight;
       uint64_t BB = gamestate.black.bishop;
        uint64_t BQ = gamestate.black.queen;
        uint64_t BK = gamestate.black.king;
        uint64_t BP = gamestate.black.pawn;
        uint64_t WR = gamestate.white.rook;
        uint64_t WN = gamestate.white.knight;
        uint64_t WB = gamestate.white.bishop;
        uint64_t WQ = gamestate.white.queen;
        uint64_t WK = gamestate.white.king;
        uint64_t WP = gamestate.white.pawn;

        bool WCK = gamestate.white.can_king_side_castle;
        bool WCQ = gamestate.white.can_queen_side_castle;
        bool BCK = gamestate.black.can_king_side_castle;
         bool BCQ = gamestate.black.can_queen_side_castle;
         bool white_move = gamestate.whites_turn;


            if (white_move) {
                get_W_moves(gamestate, E_P, CM, SM, moves);
            } else {
                get_B_moves(gamestate, E_P, CM, SM, moves);
            }


            //    nodes++; //cap_counter += cap_count_temp;
            //nodes++;
            if (depth != 0) {

                //std::cout<<


                for (int i = 0; i < moves.size(); i++) {
                    // nodes++;
                    //std::cout<<i<<std::endl;
                    int cap_count_temp = 0;

                    //   std::cout<<i<<std::endl;

//                    if (depth == 2) { std::cout << "  d1: " << moves[i] << std::endl; }
//                    if (depth == 1) { std::cout << i << "     d2: " << moves[i] << std::endl; }
//                    if (depth == 1) {
//                        if (moves[i].size() > 5 and moves[i].substr(6, 1) == "E") {
//                            std::cout << "EP" << std::endl;

                    GameState gamestate_temp;
                    memcpy(&gamestate_temp, &gamestate, sizeof(GameState));

                    uint64_t BRt = BR, BNt = BN, BBt = BB, BQt = BQ, BKt = BK, BPt = BP, WRt = WR, WNt = WN, WBt = WB, WQt = WQ, WKt = WK, WPt = WP, E_Pt = E_P;
                    bool BCKt = BCK, BCQt = BCQ, WCKt = WCK, WCQt = WCQ, CMt = false, SMt = false, white_movet = white_move;
                    if (white_movet) {
                        apply_move(white_movet, moves[i], WRt, WNt, WBt, WQt, WKt, WPt, BRt, BNt, BBt,
                                   BQt, BKt, BPt, E_Pt, WCKt, WCQt, BCKt, BCQt);
                    } else {
                        apply_move(white_movet, moves[i], BRt, BNt, BBt, BQt, BKt, BPt, WRt, WNt, WBt,
                                   WQt, WKt, WPt, E_Pt, WCKt, WCQt, BCKt, BCQt);
                    }

                        gamestate_temp.black.rook = BRt;
        gamestate_temp.black.knight = BNt;
      gamestate_temp.black.bishop = BBt;
        gamestate_temp.black.queen = BQt;
      gamestate_temp.black.king =BKt;
        gamestate_temp.black.pawn = BPt;
        gamestate_temp.white.rook = WRt;
        gamestate_temp.white.knight = WNt;
         gamestate_temp.white.bishop = WBt;
        gamestate_temp.white.queen = WQt;
        gamestate_temp.white.king = WKt;
        gamestate_temp.white.pawn = WPt;

        gamestate_temp.white.can_king_side_castle = WCKt;
        gamestate_temp.white.can_queen_side_castle = WCQt;
        gamestate_temp.black.can_king_side_castle = BCKt;
        gamestate_temp.black.can_queen_side_castle = BCQt;
        gamestate_temp.whites_turn = white_movet;

                    //  std::cout<<"depth: "<< depth<<std::endl;
                    //  std::cout<<"nodes: "<< nodes<<std::endl;

                    if (depth == 1) {
                        nodes++;
                      //  cap_counter += cap_count_temp;

                    }
                    //  else if (CMt or)
                    perft(nodes, cap_counter, gamestate_temp,
                        moves, E_Pt, CMt, SMt,
                          depth - 1, orig_depth, n);

                    if (depth == orig_depth) {
                        //          viz_bb( WPt);
                        if (n == "total") {
                        std::cout << round(((i * 100 / moves.size()))) << "% complete... -> d1:" << moves[i]
                             << "--------------------------------------------------" << std::endl;
                            }
                        else if (n == "node") {
                               std::cout<<i <<":"<<moves[i] <<" "<<nodes<<  std::endl;
                              nodes = 0;
                        }

                    }else if (depth == orig_depth - 1 and false){
                        if (n == "total") {
                            std::cout << round(((i * 100 / moves.size()))) << "% complete... -> d1:" << moves[i]
                                 << "--------------------------------------------------" << std::endl;
                        }
                        else if (n == "node") {
                            std::cout<< "     "<<i <<":"<<moves[i] <<" "<<nodes<<  std::endl;
                         //   nodes = 0;
                        }
                    }

                }
            }

}

int nodes2 = 0;

double eval(uint64_t WR, uint64_t WN, uint64_t WB, uint64_t WQ, uint64_t WK, \
            uint64_t WP, uint64_t BR, uint64_t BN, uint64_t BB, uint64_t BQ, \
            uint64_t BK, uint64_t BP){

           //material
           double counter = 0;
            counter += ((double)std::bitset<64>(WP).count() - (double)std::bitset<64>(BP).count()) * 100;
    counter += ((double)std::bitset<64>(WB).count() - (double)std::bitset<64>(BB).count()) * 300;  //todo: add special case regarding number of bishops
    counter += ((double)std::bitset<64>(WN).count() - (double)std::bitset<64>(BN).count()) * 300;
    counter += ((double)std::bitset<64>(WR).count() - (double)std::bitset<64>(BR).count()) * 500;
    counter += ((double)std::bitset<64>(WQ).count() - (double)std::bitset<64>(BQ).count()) * 900;

    return counter;

}


AI_return minimax(bool& white_move, uint64_t WR, uint64_t WN, uint64_t WB, \
                uint64_t WQ, uint64_t WK, uint64_t WP, uint64_t BR, \
                uint64_t BN, uint64_t BB, uint64_t BQ, uint64_t BK, \
                uint64_t BP, uint64_t E_P, bool WCK, bool WCQ, bool BCK, bool BCQ, bool CM, bool SM, int depth, bool my_turn, double alpha=-100000000, double beta=100000000) {


   // std::cout<<"alpha: "<<alpha<<". beta: "<<beta<<"."<<std::endl;


  //  std::cout<<depth<<std::endl;
    if (depth == 0) {  //todo: add a conditon for game over
        //todo add evaluation function
      //  std::cout<<"HEYYYYYYYYYYYY"<<std::endl;
        nodes2++;
        AI_return leaf = {"string generico", eval(WR, WN, WB, WQ, WK, WP, BR, BN, BB, BQ, BK, BP)};
    //    std::cout<<leaf.value<<std::endl;
        return leaf;
    }


    if (my_turn) {
        std::vector<std::string> w_moves;

        std::string max_move = " ";
        double max_val = -10000000;
        AI_return a;

        // TODO: uncomment this and fix
        //get_W_moves(WP, WR, WN, WB, WQ, WK, BQ, BB, BR, BN, BP, BK, E_P, WCK, WCQ, CM, SM, w_moves);
        if (CM) {// std::cout << "CHECKMATE. BLACK WINS" << std::endl;
            AI_return leaf = {"CM", -10000};
            return leaf;}
        if (SM) { //std::cout << "STALEMATE." << std::endl;
            AI_return leaf = {"SM", 0};
            return leaf;}

        for (int i = 0; i < w_moves.size(); i++) {
          //   if((depth != 1 and depth != 2) or false){std::cout << "DEPTH: " << depth << " W move " << i + 1 << ": " << w_moves[i] << std::endl;}
            //      uint64_t BRt = BR, BNt = BN, BBt = BB, BQt = BQ, BKt = BK, BPt = BP, WRt = WR, WNt = WN, WBt = WB, WQt = WQ, WKt = WK, WPt = WP, BLACK_PIECESt = BLACK_PIECES, WHITE_PIECESt = WHITE_PIECES, OCCUPIEDt , E_Pt = E_P;
            //    bool BCKt = BCK, BCQt = BCQ, WCKt = WCK, WCQt = WCQ, CMt = false, SMt = false, white_movet = white_move;
            uint64_t BRt = BR, BNt = BN, BBt = BB, BQt = BQ, BKt = BK, BPt = BP, WRt = WR, WNt = WN, WBt = WB, WQt = WQ, WKt = WK, WPt = WP, E_Pt = E_P;
            bool BCKt = BCK, BCQt = BCQ, WCKt = WCK, WCQt = WCQ, CMt = false, SMt = false, white_movet = white_move;

            apply_move(white_movet, w_moves[i], WRt, WNt, WBt, WQt, WKt, WPt, BRt, BNt, BBt,
                       BQt, BKt, BPt, E_Pt, WCKt, WCQt, BCKt, BCQt);
//
            a = minimax(white_movet, WRt, WNt, WBt, WQt, WKt, WPt, BRt, BNt, BBt, BQt, BKt, BPt, E_Pt, WCKt, WCQt, BCKt, BCQt, CMt, SMt, depth - 1, !my_turn, alpha, beta);




            if (a.value > max_val) {
                max_val = a.value;
                max_move = w_moves[i];
            }

            alpha = std::max(alpha, a.value);
            if(beta <= alpha){break;}


        }

        AI_return leaf_node;
        leaf_node.value = max_val;
        leaf_node.move = max_move;
        return leaf_node;

    } else {
        std::vector<std::string> b_moves;

        std::string min_move = " ";
        double min_val = 10000000;
        AI_return a;

        // TODO: uncomment this and fix
        //get_B_moves(BP, BR, BN, BB, BQ, BK, WQ, WB, WR, WN, WP, WK, E_P, BCK, BCQ, CM, SM, b_moves);
        if (CM) { //std::cout << "CHECKMATE. WHITE WINS" << std::endl;
            AI_return leaf = {"CM", 10000};
            return leaf;}
        if (SM) {// std::cout << "STALEMATE." << std::endl;
            AI_return leaf = {"SM", 0};
            return leaf;}

        for (int j = 0; j < b_moves.size(); j++) {
            //   if(depth == 1){nodes ++;}
       //     std::cout << "DEPTH: " << depth << " B move " << j + 1 << ": " << b_moves[j] << std::endl;


            uint64_t BRt = BR, BNt = BN, BBt = BB, BQt = BQ, BKt = BK, BPt = BP, WRt = WR, WNt = WN, WBt = WB, WQt = WQ, WKt = WK, WPt = WP, E_Pt = E_P;
            bool BCKt = BCK, BCQt = BCQ, WCKt = WCK, WCQt = WCQ, CMt = false, SMt = false, white_movet = white_move;


            apply_move(white_movet, b_moves[j], BRt, BNt, BBt, BQt, BKt, BPt, WRt, WNt, WBt,
                       WQt, WKt, WPt, E_Pt, WCKt, WCQt, BCKt, BCQt);


            a = minimax(white_movet, WRt, WNt, WBt, WQt, WKt, WPt, BRt, BNt, BBt, BQt, BKt, BPt, E_Pt, WCKt, WCQt, BCKt, BCQt, CMt, SMt, depth - 1, !my_turn, alpha, beta);
        


            if (a.value < min_val) {
                min_val = a.value;
                min_move = b_moves[j];
            }

            beta = std::min(beta, a.value);
            if(beta <= alpha){break;}
        }

        AI_return leaf_node;
        leaf_node.value = min_val;
        leaf_node.move = min_move;

        return leaf_node;
    }

        //todo: implement the eval function

    }

void fenToGameState(const std::string fen, GameState& gamestate){
    uint8_t byte = 7;
    uint8_t bit = 0;

    // Populate piece positions.
    for(int i = 0; i < fen.find(' '); i++){
        // Check for empty positions.
        if (isdigit(fen[i])){
            bit += fen[i] - '0';
        }
        switch(fen[i]) {
             case 'p':
                gamestate.black.pawn += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'r':
                gamestate.black.rook += (1ull << (byte * 8 + bit));
                bit++;
                break;
             case 'n':
                gamestate.black.knight += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'b':
                gamestate.black.bishop += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'q':
                gamestate.black.queen += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'k':
                gamestate.black.king += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'P':
                gamestate.white.pawn += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'R':
                gamestate.white.rook += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'N':
                gamestate.white.knight += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'B':
                gamestate.white.bishop += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'Q':
                gamestate.white.queen += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case 'K':
                gamestate.white.king += 1ull << (byte * 8 + bit);
                bit++;
                break;
             case '/':
                byte -= 1;
                bit = 0;
                break;
            default:
                // todo: add error catch here.
                break;
        }
    }
    

    // Populate extra game state data.
    for(int i = fen.find(' ') + 1; i < fen.length(); i++){
        switch(fen[i]){
            case 'w':
                gamestate.whites_turn = true;
                break;
            case 'b':
                gamestate.whites_turn = false;
                break;              
            case 'K':
                gamestate.white.can_king_side_castle = true;
                break;   
            case 'Q':
                gamestate.white.can_queen_side_castle = true;
                break;   
            case 'k':
                gamestate.black.can_king_side_castle = true;
                break;   
            case 'q':
                gamestate.black.can_queen_side_castle = true;
                break;   
            default:
                //todo: add error checking here.  
                break;
        }
    }
}

void read_FEN(char g[8][8], std::string FEN, bool& white_move, bool& WCK, bool& WCQ, bool& BCK, bool& BCQ){
    int row = 0;
    int col = 0;


    for (int i = 0; i < FEN.length(); i++ ){

        if(row > 7){

            //board has been read in
        //   std::cout<<to_string(FEN[i])<<std::endl;
            if(FEN[i] == 'w'){ white_move = true;}
            if(FEN[i] == 'b'){ white_move = false;}
            if(FEN[i] == 'k'){ BCK = true;}
            if(FEN[i] == 'q'){ BCQ = true;}
            if(FEN[i] == 'K'){ WCK = true;}
            if(FEN[i] == 'Q'){ WCQ = true;}



        // break;


        }else {

            if (isdigit(FEN[i])) {
            // std::cout<<FEN[i]<<std::endl;
                for (int j = 0; j < (int)(FEN[i]) - 48; j++) {
                    g[row][col] = ' ';
                //  std::cout<<row<<","<<col<<std::endl;
                    col = (col + 1) % 8;
                    if (col == 0) { row += 1; }
                    //std::cout<<g[row][col]<<std::endl;

                }

            } else if (FEN[i] == '/'){}
            else{
                g[row][col] = FEN[i];
            // std::cout<<row<<","<<col<<std::endl;
                col = (col + 1) % 8;
                if (col == 0) { row += 1; }
            }
        }

    }
} // end of readFEN

void generate_board(std::string name, int diff) {
//
    
    std::cout<<"GAME START"<<std::endl;

    char grid[8][8] ={
            //  | 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 7
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 6
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 5
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 4
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 3
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 2
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 1
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};// 0

    std::string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    //FEN = "rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1";
    //FEN= "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq d6 0 2";
    bool BCK = false, BCQ = false, WCK = false, WCQ = false, CM = false, SM = false, white_to_move;
    //todo: add E_P functionality to read FEN
    uint64_t BR = 0u, BN = 0u, BB = 0u, BQ = 0u, BK = 0u, BP = 0u, WR = 0u, WN = 0u, WB = 0u, WQ = 0u, WK = 0u, WP = 0u, BLACK_PIECES, WHITE_PIECES, OCCUPIED, E_P = 0u;//pow(2,20);;


    
    read_FEN(grid, FEN, white_to_move, WCK, WCQ, BCK, BCQ);

    grid_to_bbs(grid, BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);

    BLACK_PIECES = BR | BN | BB | BQ | BK | BP, WHITE_PIECES = WR | WN | WB | WQ | WK | WP, OCCUPIED = BLACK_PIECES | WHITE_PIECES;

    AI_return AI_choice;

    int depth;
    if(diff == 1){depth = 4;}
    else if(diff == 2){depth = 5;}
    else if(diff == 3){depth = 6;}
    
    //This is the GAME
    //for now, the AI is only white
    //todo: implement AI for both colors
    srand(time(nullptr));

    Player p = Player(true);
    Player p2 = Player(true, true);
    std::cout<<p.color<<std::endl;

    while (!CM and !SM and false){

        BLACK_PIECES = BR | BN | BB | BQ | BK | BP, WHITE_PIECES = WR | WN | WB | WQ | WK | WP, OCCUPIED = BLACK_PIECES | WHITE_PIECES;
       // c

        if(white_to_move){
            // print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);

            std::cout<<"WHITE'S MOVE: "<<std::endl;
            std::cout<<"AI Agent thinking... wait a few seconds."<<std::endl;
            auto start = std::chrono::high_resolution_clock::now();
           // std::cout<<"WHITES MOVE (SHOULD BE 1): "<<white_to_move<<std::endl;
            AI_choice = minimax(white_to_move, WR, WN, WB, WQ, WK, WP, BR, BN, BB, BQ, BK, BP, E_P, WCK, WCQ, BCK, BCQ, CM, SM, depth, true);
            auto end = std::chrono::high_resolution_clock::now();

            std::cout<<"Move chosen: "<<AI_choice.move<<std::endl;
            std::cout<<AI_choice.value<<std::endl;
            std::cout<<"WHITES MOVE (SHOULD BE 1): "<<white_to_move<<std::endl;
          //  std::cout<<"nodes: "<<nodes2<<std::endl;

            apply_move(white_to_move, AI_choice.move, WR, WN, WB, WQ, WK, WP, BR, BN, BB,
                       BQ, BK, BP, E_P, WCK, WCQ, BCK, BCQ);


            std::cout<<"depth: "<<depth<<". time elapsed: "<<(double)(end - start).count()/1000000000<<" s. nodes searched: "<<nodes2<<"."<<std::endl;
            std::cout<<"NPS: "<< nodes2/((double)(end - start).count()/1000000000)<<std::endl;
            std::cout<<" "<<std::endl;
           // break;
        }
        else{
            // print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);
            std::cout<<"BLACK'S MOVE: "<<std::endl;

            //todo: create a player class for their choosing mechanism
            std::vector<std::string> b_moves;

            // TODO: uncomment this and fix
            //get_B_moves(BP, BR, BN, BB, BQ, BK, WQ, WB, WR, WN, WP, WK, E_P, BCK, BCQ, CM, SM, b_moves);

//            if (depth == 2) {
//                if ( b_moves[j] == "63>43>2"){
//                    print_board(BRt, BNt, BBt, BQt, BKt, BPt, WRt, WNt, WBt, WQt, WKt, WPt);
//                    debug = true;
//                }else{debug = false;}
                std::cout<<"Please select your move: "<<std::endl;
                print_moves( white_to_move, b_moves, b_moves);

                int user_choice;
                std::cin >> user_choice;

                apply_move(white_to_move, b_moves[user_choice - 1], BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP, E_P, WCK, WCQ, BCK, BCQ);

                std::cout<<"Move chosen: "<<b_moves[user_choice - 1]<<std::endl;
                std::cout<<" "<<std::endl;


        }
    }

    std::vector<std::string> moves;
    if (false){




    //generate_bit();
    int turn_counter = 0, turns, game_counter = 0; std::string chosen_move;
  //  auto start = std::chrono::high_resolution_clock::now();

    srand (time(nullptr));

//todo: get rid of all pow functions. use bitshifting instead
        
      //


    turns = 0;
        for (int i = 0; i < 0; i++) {



            // TODO: uncomment when this is fixed with gamestate
            // if (white_to_move) {
            //     get_W_moves(WP, WR, WN, WB, WQ, WK, BQ, BB, BR, BN, BP, BK, E_P, WCK, WCQ, CM, SM, moves);
            // } else {
            //     get_B_moves(BP, BR, BN, BB, BQ, BK, WQ, WB, WR, WN, WP, WK, E_P, BCK, BCQ, CM, SM, moves);
            // }

            //check to see if game has ended in checkmate or stalemate
            if (CM and white_to_move) {
             //   std::cout << "CHECKMATE. BLACK WINS." << std::endl;
                break;
            }
            else if (CM and !white_to_move) {
            //    std::cout << "CHECKMATE. WHITE WINS." << std::endl;
                break;
            }
            else if (SM) {
            //    std::cout << "STALEMATE. IT'S A DRAW." << std::endl;
                break;
            }

            //print methods for validation only
           // print_moves(white_to_move, moves, moves);

            //for now, just choose a random move for each player
        //    r = (int) (rand() % moves.size());
        //    chosen_move = moves[r];
          //  std::cout<<r<<std::endl;
          //  std::cout << "MOVE #" << r + 1 << " CHOSEN." << std::endl;
            int cap_counter = 0;

            // apply the move and update the board
            if (white_to_move) {
                apply_move(white_to_move, chosen_move, WR, WN, WB, WQ, WK, WP, BR, BN, BB, BQ, BK, BP, E_P, WCK, WCQ, BCK, BCQ);
            } else {
                apply_move(white_to_move, chosen_move, BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP, E_P, WCK, WCQ, BCK, BCQ);
            }
            turns++;

            // b_moves.clear(); w_moves.clear();
           // print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);

           // moves.clear();
        }

        game_counter++;
        turn_counter += turns;

}

if (false) {
    long nodes = 0;
    std::string tests = " ";
    bool new_line = false;
    std::string FEN_;
    bool FEN_done = false;
    std::string d_num;
    int num = 0;
    for (int i = 0; i < tests.length(); i++) {
        if (tests[i] == ';' or FEN_done) {
            FEN_done = true;
            //std::cout<<tests[i]<<tests[i + 1]<<std::endl;
            if (tests[i] == 'D' and tests[i + 1] == '3') {
                for (int j = 0; j < 100; j++) {
                    // std::cout<<tests[i + 1 + 2 + j]<<std::endl;
                    if (isdigit(tests[i + 1 + 2 + j])) { d_num += tests[i + 1 + 2 + j]; }
                    else { break; }
                }
                num = stoi(d_num);
            }
        } else if (FEN_done == false) { FEN_ += tests[i]; }
        if (tests[i] == '\n') {

            std::cout << FEN_ << "-----> D3: " << d_num << std::endl;
            FEN_done = false;

            //todo: add code here

            char grid[8][8] = {
                    //  | 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 7
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 6
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 5
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 4
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 3
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 2
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 1
                    {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};// 0

            BR = 0u, BN = 0u, BB = 0u, BQ = 0u, BK = 0u, BP = 0u, WR = 0u, WN = 0u, WB = 0u, WQ = 0u, WK = 0u, WP = 0u, E_P = 0u;//pow(2,20);;


            int depth = 4;
            // std::string n = "total";
            std::string n = "total";

            // std::cout<<1<<std::endl;
            nodes = 0;
            int cap_counter = 0;
            BCK = false, BCQ = false, WCK = false, WCQ = false, CM = false, SM = false, white_to_move = true;

            //std::string FEN = FEN_;
            std::string FEN = "8/2k1p3/3pP3/3P2K1/8/8/8/8 b - - 0 1";
            // FEN = "rnRq1k1r/pp2bppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R b KQ - 0 8";
            //     FEN = "rnRq1k1r/pp2bppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R b KQ - 0 8";
            //FEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/P7/1PP1NnPP/RNBQK2R b KQ - 0 8";
            //FEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/P7/1PP1N1PP/RNBQK2n w Q - 0 9";

            read_FEN(grid, FEN, white_to_move, WCK, WCQ, BCK, BCQ);

            grid_to_bbs(grid, BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);
            // viz_bb(BR);
            //print_board(BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);

            //  BCK = true, BCQ = true, WCK = true, WCQ = true, CM = false, SM = false, check = false, white_to_move = true;

            // E_P = pow(2,21);
            // throw;
            std::cout << "HEY" << std::endl;
            // perft(nodes, cap_counter, white_to_move, WR, WN, WB, WQ, WK, WP, BR, BN, BB, BQ, BK, BP, WHITE_PIECES,
            //       BLACK_PIECES, OCCUPIED, moves, E_P, WCK, WCQ, BCK, BCQ, CM, SM, depth, depth, n);

            if (nodes != num) {
                std::cout << "generated nodes: " << nodes << std::endl;
                std::cout << "expected nodes: " << num << std::endl;

                throw;
            }

            std::cout << "depth " << depth << ":" << std::endl;
            std::cout << "total nodes: " << nodes << std::endl;
            std::cout << "total captures: " << cap_counter << std::endl;


            FEN_ = "";
            d_num = "";
        }
    }
}

if (true){

        char grid[8][8] = {
                //  | 0 |  1 |  2 |  3 |  4 |  5 |  6 |  7 |
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 7
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 6
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 5
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 4
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 3
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 2
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}, // 1
                {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}};// 0

        int depth = 3;
        // std::string n = "total";
        std::string n = "total";

        // std::cout<<1<<std::endl;
        uint32_t nodes = 0;
        uint32_t cap_counter = 0;
        //E_P = pow(2,19);
        //std::string FEN = FEN_;
         // std::string FEN = "8/3K4/2p5/p2b2r1/5k2/8/8/1q6 b - 1 67";
          std::string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
           FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
//          FEN = "8/2p5/3p4/KP5r/1R3p1k/4P3/6P1/8 b - - 0 1";
//          FEN = "8/2p5/8/KP1p3r/1R3p1k/4P3/6P1/8 w - - 0 2";
//          FEN = "8/2p5/8/KP1p3r/1R3pPk/4P3/8/8 b - g3 0 2";
        // FEN = "rnRq1k1r/pp2bppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R b KQ - 0 8";
        //     FEN = "rnRq1k1r/pp2bppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R b KQ - 0 8";
        //FEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/P7/1PP1NnPP/RNBQK2R b KQ - 0 8";
        //FEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/P7/1PP1N1PP/RNBQK2n w Q - 0 9";

        GameState gamestate;
        fenToGameState(FEN, gamestate);

        // GameState newone;
        // gamestate = newone;

        // read_FEN(grid, FEN, white_to_move, WCK, WCQ, BCK, BCQ);

        // grid_to_bbs(grid, BR, BN, BB, BQ, BK, BP, WR, WN, WB, WQ, WK, WP);
        // std::cout<< WP<<std::endl;;

        // viz_bb(BR);
        //std::cout<<"hey"<<std::endl;
        print_board(gamestate);

        //  BCK = true, BCQ = true, WCK = true, WCQ = true, CM = false, SM = false, check = false, white_to_move = true;

        // E_P = pow(2,21);
        // throw;

        perft(nodes, cap_counter, gamestate, moves, E_P, CM, SM, depth, depth, n);


        std::cout<<"depth "<<depth<<":"<<std::endl;
        std::cout<<"total nodes: "<<nodes<<std::endl;
        std::cout<<"total captureds: "<<cap_counter<<std::endl;
    }


}

//  for (int i = 0; i <10000000; i++) {
//    a = (uint64_t) 1u << 63;
// }


//    auto end = std::chrono::high_resolution_clock::now();
//    std::cout<<"bitshifting: "<<(end - start).count()/10000<<std::endl;
//
//     start = std::chrono::high_resolution_clock::now();
//
//    for (int i = 0; i <10000000; i++) {
//        a = pow(2, 63);
//    }
//    end = std::chrono::high_resolution_clock::now();
//    std::cout<<"power: "<<(end - start).count()/10000<<std::endl;




