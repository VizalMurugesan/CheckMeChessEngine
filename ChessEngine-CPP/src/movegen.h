#pragma once
#include <vector>
#include "board.h"
#include "move.h"

class MoveGenerator {

    public:
        void generateMoves(const Board& board, std::vector<Move>& moves);

    private:
        void generatePawnMoves(const Board& board, std::vector<Move>& moves);
        void generateKnightMoves(const Board& board, std::vector<Move>& moves);
        void generateBishopMoves(const Board& board, std::vector<Move>& moves);
        void generateRookMoves(const Board& board, std::vector<Move>& moves);
        void generateQueenMoves(const Board& board, std::vector<Move>& moves);
        void generateKingMoves(const Board& board, std::vector<Move>& moves);

        void generateWhitePawnMoves(const Board& board, std::vector<Move>& moves);
        void generateBlackPawnMoves(const Board& board, std::vector<Move>& moves);

        void generateWhitePawnSinglePushMoves(const Board& board, std::vector<Move>& moves);
        void generateWhitePawnDoublePushMoves(const Board& board, std::vector<Move>& moves);
        void generateWhitePawnCaptureMoves(const Board& board, std::vector<Move>& moves);

        void generateBlackPawnSinglePushMoves(const Board& board, std::vector<Move>& moves);
        void generateBlackPawnDoublePushMoves(const Board& board, std::vector<Move>& moves);
        void generateBlackPawnCaptureMoves(const Board& board, std::vector<Move>& moves);

        void generateCastlingMoves(const Board& board, std::vector<Move>& moves);
        void generateEnPassantMoves(const Board& board, std::vector<Move>& moves);

        bool isSquareAttacked(const Board& board, int sq, Color attacker) const;
        bool isInCheck(const Board& board, Color side) const;
        void filterIllegalMoves(const Board& board, std::vector<Move>& moves);
};