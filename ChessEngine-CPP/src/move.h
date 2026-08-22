#pragma once
#include<cstdint>
#include "board.h"

enum MoveType { NORMAL, PROMOTION, CASTLING, EN_PASSANT, CAPTURE };

struct Move{

    uint32_t data;

    // bits 0-5:   from square
    // bits 6-11:  to square
    // bits 12-14: piece type
    // bits 15-17: move type
    // bits 18-20: promotion piece type (only meaningful when type == PROMOTION)
    Move(uint32_t from, uint32_t to, PieceType pieceType, MoveType type, PieceType promoteTo = QUEEN) {
        data = (from & 0x3F) | ((to & 0x3F) << 6) | ((pieceType & 0x7) << 12)
             | ((type & 0x7) << 15) | ((promoteTo & 0x7) << 18);
    };

    inline uint32_t getFrom() const {
        return data & 0x3F;
    }

    inline uint32_t getTo() const {
        return (data >> 6) & 0x3F;
    }

    inline PieceType getPieceType() const {
        return static_cast<PieceType>((data >> 12) & 0x7);
    }

    inline MoveType getMoveType() const {
        return static_cast<MoveType>((data >> 15) & 0x7);
    }

    inline PieceType getPromoteTo() const {
        return static_cast<PieceType>((data >> 18) & 0x7);
    }

    // 1. Default constructor (needed for uninitialized move variables, root PV, TT entries)
    constexpr Move() : data(0) {}

    // 2. Convenience boolean checks
    inline bool isCapture() const {
        MoveType t = getMoveType();
        return t == CAPTURE || t == EN_PASSANT;
    }

    inline bool isPromotion() const {
        return getMoveType() == PROMOTION;
    }

    inline bool isNull() const {
        return data == 0;
    }

    // 3. Equality operators (needed for comparing best move, transposition tables, tests)
    inline bool operator==(const Move& other) const { return data == other.data; }
    inline bool operator!=(const Move& other) const { return data != other.data; }

};