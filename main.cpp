/*
 * COVID Chess Engine
 * developed by David Doellstedt
 *
 * Instructions:
 * Run the main.cpp file. This will start the game.
 * Enter your name and your desired difficulty for the AI Agent.
 * To take your turn, enter a number into the command line, equaling one of your
 * possible moves printed on the console. Then, wait for the AI to move.
 *
 * The game will continue until a checkmate or stalemate occurs. Goodluck!
 *
 * Notes:
 *      capital letters = WHITE
 *      lowercase letters = BLACK
 *      n = knight
 *
 *
 * Background:
 * --------------------------
 * I developed this chess engine as a way to practice my algorithms and get some
 * more experience doing AI projects. I took an AI course at Georgia Tech in
 * early 2020. I took some concepts learned in the course and scaled them up to
 * tackle a more challenging game, chess. I learned a TON about C++ (my first
 * C++ project), bit manipulation, recursion, algorithm optimization,
 * memoization.
 *
 * I first started in Python. A language I know pretty well. I developed the
 * entire program in about 50 hours, with a basic evaluation function. I took an
 * OOP approach, Using a board object and piece objects to achieve my possible
 * move generation. I realized that if I wanted to get an AI worth i's salt, I
 * needed to optimize my program, a lot and switch languages too. I wasn't able
 * to search very deep at all in my python OOP method.
 *
 * I decided to switch to C++ to vastly improve my AI performance and to get
 * experience with C++ (I had none at the time) I switched approaches too. I
 * decide to represent the gamestate much more memory efficiently and with a lot
 * less garbage clean up required when I add or remove objects into the game.
 * (See Game state below). Presently, I've put about 100 hours into this
 * program. I have a few things more I'd like to do:
 *      - Add memoization
 *      - Add iterative deepening
 *      - Clean up my class structure
 *      - Improve my evaluation function (currently it is a fairly naive
 * evaluation based on Material weight)
 *      - Create a nice GUI
 *      - add a P2P functionality and an AI2AI simulation
 *      - Add this chess game to an app or website
 *
 * (This is a never ending project of sorts. I am progressing it when I have
 * time and motivation to work on it.)
 *
 *
 * Key Features of my engine:
 * --------------------------
 *
 * Game state:
 * The entire game state as a set of 12, 64 bit unsigned long longs (bitobard
 * for the 8x8 chess board).
 *    - 1 64 bit bitboard per each unique type of piece, 12 total
 *    - Some flags are also needed to fully represent the game state, such as
 * castling flags, en passant flags, check flags, etc.
 *
 * Algorithm:
 * The AI Agent decides the best move with a minimax game tree. I use alpha beta
 * pruning to narrow my tree, and search deeper. I'm able to search 6 levels
 * deep (in about 3 to 5 seconds at the start of the game, which is typically
 * where we see the highest branching factor). I think I can add 2 to 3 levels
 * with some general code optimizations and memoization. I need to implement
 * Iterative Deepening into my algorithm. This will be especially important in
 * end-game, where the branching factor is significantly reduced.
 *
 * Legal Move Generation:
 * I spent by far the most amount of time generating all legal moves. Everything
 * is calculated using bit manipulation and bit calculations. It really pushed
 * me to the limit (in a good way) as I expanded my knowledge of binary
 * calculations and bitboards. I spent about 20 hours debugging and verifying
 * corner cases in order to create a perfect legal move generator. I did this
 * through recursive perft tests where I generate the number of possible moves
 * for each move, to a certain depth. Once you have this information, you can
 * compare the number against the proven number from your source. If there are
 * discrepancies, then you must search the game tree in those specific error
 * moves and continue searching down the tree until you arrive at your error
 * source.There are many resources online for this.
 *
 *
 * Thanks for reading the story and I hope you enjoy!
 * It ain't much, but it's honest work.
 */

#include "bit_fns.h"
#include "unit_tests.h"
#include <iostream>
#include <string>

int main() {

  // string player_name;
  // int difficulty;
  // std::cout<<"------------------------------------------------------------------------"<<endl;
  // cout<<"   David Doellstedt's Chess Engine. Created during Quarantine
  // 2020."<<endl; cout<<"(My evaluation function needs A LOT of work. Don't
  // judge my AI, they are still learning :) )"<<endl;
  // cout<<"------------------------------------------------------------------------"<<endl;
  // cout<<"What is your name?"<<endl;
  // cin>>player_name;
  // cout<<"Select an AI skill level (enter a number 1, 2, or 3) "<<endl;
  // cout<<"1: Easy"<<endl;
  // cout<<"2: Medium"<<endl;
  // cout<<"3: Hard"<<endl;
  // cin>>difficulty;

  uint8_t depth = 3;
  std::string fen =
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
  // fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  // fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -";

  int result = legalMoveGeneratorTest(fen, depth);
  if (result == 97862) {
    std::cout << "YAY IT PASSED" << std::endl;
  } else {
    std::cout << "FAILED" << std::endl;
  }
  std::cout << result << std::endl;

  //   generate_board("a", 1);
  return 0;
}
