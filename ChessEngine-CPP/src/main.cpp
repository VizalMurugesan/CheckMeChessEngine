#include <iostream>
#include <string>
#include "board.h"
#include "movegen.h"

static const char* pieceNames[] = { "Pawn", "Knight", "Bishop", "Rook", "Queen", "King" };
static const char* moveTypeNames[] = { "Normal", "Promotion", "Castling", "En Passant", "Capture" };

int main() {
    Board board;
    MoveGenerator gen;
    std::vector<Move> moves;

    gen.generateMoves(board, moves);

    std::cout << "Legal moves for White at starting position: " << moves.size() << "\n\n";
    for (const Move& m : moves) {
        std::cout << pieceNames[m.getPieceType()]
                  << " " << board.squareName(m.getFrom())
                  << " -> " << board.squareName(m.getTo())
                  << " [" << moveTypeNames[m.getMoveType()] << "]";
        if (m.getMoveType() == PROMOTION)
            std::cout << " => " << pieceNames[m.getPromoteTo()];
        std::cout << "\n";
    }

    return 0;
}
