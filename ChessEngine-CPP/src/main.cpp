#include <iostream>
#include <string>
#include "board.h"
#include "movegen.h"

static const char* pieceNames[] = { "Pawn", "Knight", "Bishop", "Rook", "Queen", "King" };
static const char* moveTypeNames[] = { "Normal", "Promotion", "Castling", "En Passant", "Capture" };

static std::string squareName(int sq) {
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return std::string(1, file) + rank;
}

int main() {
    Board board;
    MoveGenerator gen;
    std::vector<Move> moves;

    gen.generateMoves(board, moves);

    std::cout << "Legal moves for White at starting position: " << moves.size() << "\n\n";
    for (const Move& m : moves) {
        std::cout << pieceNames[m.getPieceType()]
                  << " " << squareName(m.getFrom())
                  << " -> " << squareName(m.getTo())
                  << " [" << moveTypeNames[m.getMoveType()] << "]";
        if (m.getMoveType() == PROMOTION)
            std::cout << " => " << pieceNames[m.getPromoteTo()];
        std::cout << "\n";
    }

    return 0;
}
