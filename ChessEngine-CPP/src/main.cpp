#include <iostream>
#include <string>
#include "board.h"
#include "movegen.h"



int main(int argc, char* argv[]) {
    Board board;
    if(argc>1){
        std::string fen = argv[1];
        board.setFromFEN(fen);
    }
    MoveGenerator gen;
    std::vector<Move> moves;

    gen.generateMoves(board, moves);

    for (const Move& move : moves) {
        std::cout << board.squareName(move.getFrom()) << board.squareName(move.getTo()) << std::endl;
    }
    
    

    return 0;
}
