#pragma once
#include <cstdint>
#include <string>

enum Color {WHITE = 0, BLACK = 1};
enum PieceType {PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5};


struct Board {

    uint64_t pieces[2][6];
    Color turn;
    uint8_t castlingRights;   // bit 0: WK, bit 1: WQ, bit 2: BK, bit 3: BQ
    int8_t  enPassantSquare;  // -1 if none, otherwise target square index (0-63)

    Board(){
        pieces[WHITE][PAWN]   = 0x000000000000FF00;  // rank 2
        pieces[WHITE][ROOK]   = 0x0000000000000081;  // a1 and h1
        pieces[WHITE][KNIGHT] = 0x0000000000000042;  // b1 and g1
        pieces[WHITE][BISHOP] = 0x0000000000000024;  // c1 and f1
        pieces[WHITE][QUEEN]  = 0x0000000000000008;  // d1
        pieces[WHITE][KING]   = 0x0000000000000010;  // e1
        pieces[BLACK][PAWN]   = 0x00FF000000000000;  // rank 7
        pieces[BLACK][ROOK]   = 0x8100000000000000;  // a8 and h8
        pieces[BLACK][KNIGHT] = 0x4200000000000000;  // b8 and g8
        pieces[BLACK][BISHOP] = 0x2400000000000000;  // c8 and f8
        pieces[BLACK][QUEEN]  = 0x0800000000000000;  // d8
        pieces[BLACK][KING]   = 0x1000000000000000;  // e8

        turn = WHITE;
        castlingRights  = 0x0F;
        enPassantSquare = -1;
    }

    inline uint64_t whitePieces() const {
        return pieces[WHITE][PAWN] | pieces[WHITE][KNIGHT] | pieces[WHITE][BISHOP] |
               pieces[WHITE][ROOK] | pieces[WHITE][QUEEN] | pieces[WHITE][KING];
    }

    inline uint64_t blackPieces() const {
        return pieces[BLACK][PAWN] | pieces[BLACK][KNIGHT] | pieces[BLACK][BISHOP] |
               pieces[BLACK][ROOK] | pieces[BLACK][QUEEN] | pieces[BLACK][KING];
    }

    inline uint64_t occupied() const {
        return whitePieces() | blackPieces();
    }

    std :: string squareName(int sq) const{
        char file = 'a' + (sq % 8);
        char rank = '1' + (sq / 8);
        return std::string(1, file) + std::string(1, rank);
    }
};