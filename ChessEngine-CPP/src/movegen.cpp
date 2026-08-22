#include "movegen.h"
#include <algorithm>

#ifdef _MSC_VER
#include <intrin.h>
static inline int __builtin_ctzll(unsigned long long x) {
    unsigned long index;
    _BitScanForward64(&index, x);
    return static_cast<int>(index);
}
#endif

static constexpr uint64_t FILE_A = 0x0101010101010101ULL;
static constexpr uint64_t FILE_B = 0x0202020202020202ULL;
static constexpr uint64_t FILE_G = 0x4040404040404040ULL;
static constexpr uint64_t FILE_H = 0x8080808080808080ULL;
static constexpr uint64_t RANK_1 = 0x00000000000000FFULL;
static constexpr uint64_t RANK_8 = 0xFF00000000000000ULL;

void MoveGenerator::generateMoves(const Board& board, std::vector<Move>& moves) {
    generatePseudoLegalMoves(board, moves);
    filterIllegalMoves(board, moves);
}

void MoveGenerator::generatePseudoLegalMoves(const Board& board, std::vector<Move>& moves) {
    generatePawnMoves(board, moves);
    generateKnightMoves(board, moves);
    generateBishopMoves(board, moves);
    generateRookMoves(board, moves);
    generateQueenMoves(board, moves);
    generateKingMoves(board, moves);
    generateCastlingMoves(board, moves);
    generateEnPassantMoves(board, moves);
    // No filterIllegalMoves() here — that's the whole point.
}



void MoveGenerator::generateKnightMoves(const Board& board, std::vector<Move>& moves) {
    Color side = board.turn;
    uint64_t knights = board.pieces[side][KNIGHT];
    uint64_t ownPieces    = (side == WHITE) ? board.whitePieces() : board.blackPieces();
    uint64_t enemyPieces  = (side == WHITE) ? board.blackPieces() : board.whitePieces();

    while (knights != 0) {
        int sq = __builtin_ctzll(knights);
        knights &= knights - 1;
        uint64_t bit = 1ULL << sq;

        uint64_t attacks = 0;
        attacks |= (bit & ~FILE_H)            << 17; // NNE
        attacks |= (bit & ~FILE_A)            << 15; // NNW
        attacks |= (bit & ~FILE_G & ~FILE_H)  << 10; // ENE
        attacks |= (bit & ~FILE_A & ~FILE_B)  << 6;  // WNW
        attacks |= (bit & ~FILE_A)            >> 17; // SSW
        attacks |= (bit & ~FILE_H)            >> 15; // SSE
        attacks |= (bit & ~FILE_A & ~FILE_B)  >> 10; // WSW
        attacks |= (bit & ~FILE_G & ~FILE_H)  >> 6;  // ESE
        attacks &= ~ownPieces;

        while (attacks != 0) {
            int toSq = __builtin_ctzll(attacks);
            attacks &= attacks - 1;
            MoveType mt = ((enemyPieces >> toSq) & 1) ? CAPTURE : NORMAL;
            moves.push_back(Move(sq, toSq, KNIGHT, mt));
        }
    }
}

void MoveGenerator::generateBishopMoves(const Board& board, std::vector<Move>& moves) {
    Color side = board.turn;
    uint64_t bishops = board.pieces[side][BISHOP];
    uint64_t ownPieces   = (side == WHITE) ? board.whitePieces() : board.blackPieces();
    uint64_t enemyPieces = (side == WHITE) ? board.blackPieces() : board.whitePieces();

    while (bishops != 0) {
        int sq = __builtin_ctzll(bishops);
        bishops &= bishops - 1;
        uint64_t ray;

        // NE (+9): stop at file H or rank 8
        ray = 1ULL << sq;
        while (ray & ~FILE_H & ~RANK_8) {
            ray <<= 9;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), BISHOP, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        // NW (+7): stop at file A or rank 8
        ray = 1ULL << sq;
        while (ray & ~FILE_A & ~RANK_8) {
            ray <<= 7;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), BISHOP, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        // SE (-7): stop at file H or rank 1
        ray = 1ULL << sq;
        while (ray & ~FILE_H & ~RANK_1) {
            ray >>= 7;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), BISHOP, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        // SW (-9): stop at file A or rank 1
        ray = 1ULL << sq;
        while (ray & ~FILE_A & ~RANK_1) {
            ray >>= 9;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), BISHOP, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
    }
}

void MoveGenerator::generateRookMoves(const Board& board, std::vector<Move>& moves) {
    Color side = board.turn;
    uint64_t rooks = board.pieces[side][ROOK];
    uint64_t ownPieces   = (side == WHITE) ? board.whitePieces() : board.blackPieces();
    uint64_t enemyPieces = (side == WHITE) ? board.blackPieces() : board.whitePieces();

    while (rooks != 0) {
        int sq = __builtin_ctzll(rooks);
        rooks &= rooks - 1;
        uint64_t ray;

        // North (+8)
        ray = 1ULL << sq;
        while (ray & ~RANK_8) {
            ray <<= 8;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), ROOK, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        // South (-8)
        ray = 1ULL << sq;
        while (ray & ~RANK_1) {
            ray >>= 8;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), ROOK, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        // East (+1)
        ray = 1ULL << sq;
        while (ray & ~FILE_H) {
            ray <<= 1;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), ROOK, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        // West (-1)
        ray = 1ULL << sq;
        while (ray & ~FILE_A) {
            ray >>= 1;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), ROOK, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
    }
}

void MoveGenerator::generateQueenMoves(const Board& board, std::vector<Move>& moves) {
    Color side = board.turn;
    uint64_t queens = board.pieces[side][QUEEN];
    uint64_t ownPieces   = (side == WHITE) ? board.whitePieces() : board.blackPieces();
    uint64_t enemyPieces = (side == WHITE) ? board.blackPieces() : board.whitePieces();

    while (queens != 0) {
        int sq = __builtin_ctzll(queens);
        queens &= queens - 1;
        uint64_t ray;

        // Diagonals (bishop-like)
        ray = 1ULL << sq;
        while (ray & ~FILE_H & ~RANK_8) {
            ray <<= 9;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), QUEEN, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        ray = 1ULL << sq;
        while (ray & ~FILE_A & ~RANK_8) {
            ray <<= 7;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), QUEEN, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        ray = 1ULL << sq;
        while (ray & ~FILE_H & ~RANK_1) {
            ray >>= 7;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), QUEEN, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        ray = 1ULL << sq;
        while (ray & ~FILE_A & ~RANK_1) {
            ray >>= 9;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), QUEEN, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        // Straight lines (rook-like)
        ray = 1ULL << sq;
        while (ray & ~RANK_8) {
            ray <<= 8;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), QUEEN, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        ray = 1ULL << sq;
        while (ray & ~RANK_1) {
            ray >>= 8;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), QUEEN, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        ray = 1ULL << sq;
        while (ray & ~FILE_H) {
            ray <<= 1;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), QUEEN, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
        ray = 1ULL << sq;
        while (ray & ~FILE_A) {
            ray >>= 1;
            if (ray & ownPieces) break;
            moves.push_back(Move(sq, __builtin_ctzll(ray), QUEEN, (ray & enemyPieces) ? CAPTURE : NORMAL));
            if (ray & enemyPieces) break;
        }
    }
}

void MoveGenerator::generateKingMoves(const Board& board, std::vector<Move>& moves) {
    Color side = board.turn;
    uint64_t king = board.pieces[side][KING];
    uint64_t ownPieces   = (side == WHITE) ? board.whitePieces() : board.blackPieces();
    uint64_t enemyPieces = (side == WHITE) ? board.blackPieces() : board.whitePieces();

    int sq = __builtin_ctzll(king);

    uint64_t attacks = 0;
    attacks |= (king & ~RANK_8)            << 8;  // N
    attacks |= (king & ~RANK_1)            >> 8;  // S
    attacks |= (king & ~FILE_H)            << 1;  // E
    attacks |= (king & ~FILE_A)            >> 1;  // W
    attacks |= (king & ~FILE_H & ~RANK_8)  << 9;  // NE
    attacks |= (king & ~FILE_A & ~RANK_8)  << 7;  // NW
    attacks |= (king & ~FILE_H & ~RANK_1)  >> 7;  // SE
    attacks |= (king & ~FILE_A & ~RANK_1)  >> 9;  // SW
    attacks &= ~ownPieces;

    while (attacks != 0) {
        int toSq = __builtin_ctzll(attacks);
        attacks &= attacks - 1;
        MoveType mt = ((enemyPieces >> toSq) & 1) ? CAPTURE : NORMAL;
        moves.push_back(Move(sq, toSq, KING, mt));
    }
}

// PAWN MOVES CALCULATION
#pragma region 

void MoveGenerator::generatePawnMoves(const Board& board, std::vector<Move>& moves) {
    if (board.turn == WHITE) {
        generateWhitePawnMoves(board, moves);
    } else {
        generateBlackPawnMoves(board, moves);
    }
}

void MoveGenerator::generateWhitePawnMoves(const Board& board, std::vector<Move>& moves) {
    generateWhitePawnSinglePushMoves(board, moves);
    generateWhitePawnDoublePushMoves(board, moves);
    generateWhitePawnCaptureMoves(board, moves);
}

void MoveGenerator::generateWhitePawnSinglePushMoves(const Board& board, std::vector<Move>& moves) {
    uint64_t whitePawns = board.pieces[WHITE][PAWN];
    uint64_t emptySquares = ~board.occupied();
    uint64_t targetSquares = (whitePawns << 8) & emptySquares;

    while (targetSquares != 0) {
        int toSquare   = __builtin_ctzll(targetSquares);
        targetSquares &= targetSquares - 1;
        int fromSquare = toSquare - 8;
        if (toSquare >= 56) { // Reached rank 8 — promote
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, QUEEN));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, ROOK));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, BISHOP));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, KNIGHT));
        } else {
            moves.push_back(Move(fromSquare, toSquare, PAWN, NORMAL));
        }
    }
}

void MoveGenerator::generateWhitePawnDoublePushMoves(const Board& board, std::vector<Move>& moves) {
    uint64_t whitePawns = board.pieces[WHITE][PAWN];
    uint64_t emptySquares = ~board.occupied();
    uint64_t targetSquares = ((whitePawns << 16) & emptySquares) & (emptySquares << 8); // Double move forward

    while (targetSquares!=0){

        int toSquare = __builtin_ctzll(targetSquares); // Get the index of the least significant bit
        targetSquares &= targetSquares - 1; // Clear the least significant bit
        int fromSquare = toSquare - 16; // Calculate the from square index
        if(fromSquare>=8 && fromSquare<16){
            moves.push_back(Move(fromSquare, toSquare, PAWN, NORMAL)); // Add the move to the list
        } 
        

    }
}

void MoveGenerator::generateBlackPawnMoves(const Board& board, std::vector<Move>& moves) {
    generateBlackPawnSinglePushMoves(board, moves);
    generateBlackPawnDoublePushMoves(board, moves);
    generateBlackPawnCaptureMoves(board, moves);
}

void MoveGenerator::generateBlackPawnSinglePushMoves(const Board& board, std::vector<Move>& moves) {
    uint64_t blackPawns = board.pieces[BLACK][PAWN];
    uint64_t emptySquares = ~board.occupied();
    uint64_t targetSquares = (blackPawns >> 8) & emptySquares;

    while (targetSquares != 0) {
        int toSquare   = __builtin_ctzll(targetSquares);
        targetSquares &= targetSquares - 1;
        int fromSquare = toSquare + 8;
        if (toSquare < 8) { // Reached rank 1 — promote
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, QUEEN));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, ROOK));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, BISHOP));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, KNIGHT));
        } else {
            moves.push_back(Move(fromSquare, toSquare, PAWN, NORMAL));
        }
    }
}

void MoveGenerator::generateBlackPawnDoublePushMoves(const Board& board, std::vector<Move>& moves) {
    uint64_t blackPawns = board.pieces[BLACK][PAWN];
    uint64_t emptySquares = ~board.occupied();
    uint64_t targetSquares = (blackPawns >> 16) & emptySquares & (emptySquares >> 8); // Double move forward

    while (targetSquares != 0) {
        int toSquare = __builtin_ctzll(targetSquares);
        targetSquares &= targetSquares - 1;
        int fromSquare = toSquare + 16;
        if (fromSquare >= 48 && fromSquare < 56) { // Must originate from rank 7
            moves.push_back(Move(fromSquare, toSquare, PAWN, NORMAL));
        }
    }
}

void MoveGenerator::generateWhitePawnCaptureMoves(const Board& board, std::vector<Move>& moves) {
    uint64_t whitePawns = board.pieces[WHITE][PAWN];
    uint64_t blackPieces = board.blackPieces();

    // Capture north-west (+7): exclude a-file pawns to prevent wrap
    uint64_t nwCaptures = (whitePawns & ~FILE_A) << 7 & blackPieces;
    while (nwCaptures != 0) {
        int toSquare   = __builtin_ctzll(nwCaptures);
        nwCaptures    &= nwCaptures - 1;
        int fromSquare = toSquare - 7;
        if (toSquare >= 56) {
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, QUEEN));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, ROOK));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, BISHOP));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, KNIGHT));
        } else {
            moves.push_back(Move(fromSquare, toSquare, PAWN, CAPTURE));
        }
    }

    // Capture north-east (+9): exclude h-file pawns to prevent wrap
    uint64_t neCaptures = (whitePawns & ~FILE_H) << 9 & blackPieces;
    while (neCaptures != 0) {
        int toSquare   = __builtin_ctzll(neCaptures);
        neCaptures    &= neCaptures - 1;
        int fromSquare = toSquare - 9;
        if (toSquare >= 56) {
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, QUEEN));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, ROOK));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, BISHOP));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, KNIGHT));
        } else {
            moves.push_back(Move(fromSquare, toSquare, PAWN, CAPTURE));
        }
    }
}

void MoveGenerator::generateBlackPawnCaptureMoves(const Board& board, std::vector<Move>& moves) {
    uint64_t blackPawns = board.pieces[BLACK][PAWN];
    uint64_t whitePieces = board.whitePieces();

    // Capture south-west (-9): exclude a-file pawns to prevent wrap
    uint64_t swCaptures = (blackPawns & ~FILE_A) >> 9 & whitePieces;
    while (swCaptures != 0) {
        int toSquare   = __builtin_ctzll(swCaptures);
        swCaptures    &= swCaptures - 1;
        int fromSquare = toSquare + 9;
        if (toSquare < 8) {
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, QUEEN));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, ROOK));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, BISHOP));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, KNIGHT));
        } else {
            moves.push_back(Move(fromSquare, toSquare, PAWN, CAPTURE));
        }
    }

    // Capture south-east (-7): exclude h-file pawns to prevent wrap
    uint64_t seCaptures = (blackPawns & ~FILE_H) >> 7 & whitePieces;
    while (seCaptures != 0) {
        int toSquare   = __builtin_ctzll(seCaptures);
        seCaptures    &= seCaptures - 1;
        int fromSquare = toSquare + 7;
        if (toSquare < 8) {
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, QUEEN));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, ROOK));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, BISHOP));
            moves.push_back(Move(fromSquare, toSquare, PAWN, PROMOTION, KNIGHT));
        } else {
            moves.push_back(Move(fromSquare, toSquare, PAWN, CAPTURE));
        }
    }
}

#pragma endregion

// ─── helper: apply a move to a board copy for legality checking ──────────────
static void applyMoveToBoard(Board& b, const Move& m) {
    int       from  = static_cast<int>(m.getFrom());
    int       to    = static_cast<int>(m.getTo());
    PieceType pt    = m.getPieceType();
    MoveType  mt    = m.getMoveType();
    Color     side  = b.turn;
    Color     enemy = (side == WHITE) ? BLACK : WHITE;

    // Lift the moving piece
    b.pieces[side][pt] &= ~(1ULL << from);

    // Remove any enemy piece on the destination (regular capture)
    if (mt == CAPTURE) {
        for (int p = 0; p < 6; p++)
            b.pieces[enemy][p] &= ~(1ULL << to);
    }

    // Place piece (or promoted piece) on destination
    if (mt == PROMOTION) {
        b.pieces[side][m.getPromoteTo()] |= (1ULL << to);
    } else {
        b.pieces[side][pt] |= (1ULL << to);
    }

    // En passant: remove the captured pawn behind the target square
    if (mt == EN_PASSANT) {
        int capSq = (side == WHITE) ? (to - 8) : (to + 8);
        b.pieces[enemy][PAWN] &= ~(1ULL << capSq);
    }

    // Castling: also move the rook
    if (mt == CASTLING) {
        if      (to == 6)  { b.pieces[WHITE][ROOK] &= ~(1ULL<<7);  b.pieces[WHITE][ROOK] |= (1ULL<<5);  } // WK
        else if (to == 2)  { b.pieces[WHITE][ROOK] &= ~(1ULL<<0);  b.pieces[WHITE][ROOK] |= (1ULL<<3);  } // WQ
        else if (to == 62) { b.pieces[BLACK][ROOK] &= ~(1ULL<<63); b.pieces[BLACK][ROOK] |= (1ULL<<61); } // BK
        else if (to == 58) { b.pieces[BLACK][ROOK] &= ~(1ULL<<56); b.pieces[BLACK][ROOK] |= (1ULL<<59); } // BQ
    }

    b.turn = enemy;
}

// ─── isSquareAttacked ─────────────────────────────────────────────────────────
bool MoveGenerator::isSquareAttacked(const Board& board, int sq, Color attacker) const {
    uint64_t occ   = board.occupied();
    uint64_t sqBit = 1ULL << sq;

    // Pawn attacks (reverse-trace from sq)
    if (attacker == WHITE) {
        uint64_t pawns = ((sqBit & ~FILE_H) >> 7) | ((sqBit & ~FILE_A) >> 9);
        if (pawns & board.pieces[WHITE][PAWN]) return true;
    } else {
        uint64_t pawns = ((sqBit & ~FILE_A) << 7) | ((sqBit & ~FILE_H) << 9);
        if (pawns & board.pieces[BLACK][PAWN]) return true;
    }

    // Knight attacks
    uint64_t kn = 0;
    kn |= (sqBit & ~FILE_H)           << 17;
    kn |= (sqBit & ~FILE_A)           << 15;
    kn |= (sqBit & ~FILE_G & ~FILE_H) << 10;
    kn |= (sqBit & ~FILE_A & ~FILE_B) << 6;
    kn |= (sqBit & ~FILE_A)           >> 17;
    kn |= (sqBit & ~FILE_H)           >> 15;
    kn |= (sqBit & ~FILE_A & ~FILE_B) >> 10;
    kn |= (sqBit & ~FILE_G & ~FILE_H) >> 6;
    if (kn & board.pieces[attacker][KNIGHT]) return true;

    // Diagonal sliders (bishop / queen)
    uint64_t diag = board.pieces[attacker][BISHOP] | board.pieces[attacker][QUEEN];
    uint64_t ray;
    ray = sqBit; while (ray & ~FILE_H & ~RANK_8) { ray <<= 9; if (ray & occ) { if (ray & diag) return true; break; } }
    ray = sqBit; while (ray & ~FILE_A & ~RANK_8) { ray <<= 7; if (ray & occ) { if (ray & diag) return true; break; } }
    ray = sqBit; while (ray & ~FILE_H & ~RANK_1) { ray >>= 7; if (ray & occ) { if (ray & diag) return true; break; } }
    ray = sqBit; while (ray & ~FILE_A & ~RANK_1) { ray >>= 9; if (ray & occ) { if (ray & diag) return true; break; } }

    // Straight sliders (rook / queen)
    uint64_t orth = board.pieces[attacker][ROOK] | board.pieces[attacker][QUEEN];
    ray = sqBit; while (ray & ~RANK_8) { ray <<= 8; if (ray & occ) { if (ray & orth) return true; break; } }
    ray = sqBit; while (ray & ~RANK_1) { ray >>= 8; if (ray & occ) { if (ray & orth) return true; break; } }
    ray = sqBit; while (ray & ~FILE_H) { ray <<= 1; if (ray & occ) { if (ray & orth) return true; break; } }
    ray = sqBit; while (ray & ~FILE_A) { ray >>= 1; if (ray & occ) { if (ray & orth) return true; break; } }

    // King
    uint64_t kg = 0;
    kg |= (sqBit & ~RANK_8)           << 8;
    kg |= (sqBit & ~RANK_1)           >> 8;
    kg |= (sqBit & ~FILE_H)           << 1;
    kg |= (sqBit & ~FILE_A)           >> 1;
    kg |= (sqBit & ~FILE_H & ~RANK_8) << 9;
    kg |= (sqBit & ~FILE_A & ~RANK_8) << 7;
    kg |= (sqBit & ~FILE_H & ~RANK_1) >> 7;
    kg |= (sqBit & ~FILE_A & ~RANK_1) >> 9;
    if (kg & board.pieces[attacker][KING]) return true;

    return false;
}

bool MoveGenerator::isInCheck(const Board& board, Color side) const {
    int kingSq = __builtin_ctzll(board.pieces[side][KING]);
    return isSquareAttacked(board, kingSq, (side == WHITE) ? BLACK : WHITE);
}

// ─── filterIllegalMoves ───────────────────────────────────────────────────────
void MoveGenerator::filterIllegalMoves(const Board& board, std::vector<Move>& moves) {
    Color side = board.turn;
    moves.erase(
        std::remove_if(moves.begin(), moves.end(), [&](const Move& m) {
            Board copy = board;
            applyMoveToBoard(copy, m);
            return isInCheck(copy, side);
        }),
        moves.end()
    );
}

// ─── generateCastlingMoves ────────────────────────────────────────────────────
void MoveGenerator::generateCastlingMoves(const Board& board, std::vector<Move>& moves) {
    Color    side  = board.turn;
    Color    enemy = (side == WHITE) ? BLACK : WHITE;
    uint64_t occ   = board.occupied();

    if (side == WHITE) {
        // Kingside: e1(4)→g1(6), f1(5) and g1(6) must be empty
        if ((board.castlingRights & 0x1) &&
            !(occ & 0x60ULL) &&
            !isSquareAttacked(board, 4, enemy) &&
            !isSquareAttacked(board, 5, enemy))
        {
            moves.push_back(Move(4, 6, KING, CASTLING));
        }
        // Queenside: e1(4)→c1(2), b1(1) c1(2) d1(3) must be empty
        if ((board.castlingRights & 0x2) &&
            !(occ & 0xEULL) &&
            !isSquareAttacked(board, 4, enemy) &&
            !isSquareAttacked(board, 3, enemy))
        {
            moves.push_back(Move(4, 2, KING, CASTLING));
        }
    } else {
        // Kingside: e8(60)→g8(62), f8(61) and g8(62) must be empty
        if ((board.castlingRights & 0x4) &&
            !(occ & 0x6000000000000000ULL) &&
            !isSquareAttacked(board, 60, enemy) &&
            !isSquareAttacked(board, 61, enemy))
        {
            moves.push_back(Move(60, 62, KING, CASTLING));
        }
        // Queenside: e8(60)→c8(58), b8(57) c8(58) d8(59) must be empty
        if ((board.castlingRights & 0x8) &&
            !(occ & 0x0E00000000000000ULL) &&
            !isSquareAttacked(board, 60, enemy) &&
            !isSquareAttacked(board, 59, enemy))
        {
            moves.push_back(Move(60, 58, KING, CASTLING));
        }
    }
}

// ─── generateEnPassantMoves ───────────────────────────────────────────────────
void MoveGenerator::generateEnPassantMoves(const Board& board, std::vector<Move>& moves) {
    if (board.enPassantSquare == -1) return;
    int      epSq  = board.enPassantSquare;
    uint64_t epBit = 1ULL << epSq;
    Color    side  = board.turn;

    if (side == WHITE) {
        // Pawn at epSq-9 captures NE (+9) to epSq: valid if epSq not on FILE_A
        if (!(epBit & FILE_A)) {
            int fromSq = epSq - 9;
            if (fromSq >= 0 && ((board.pieces[WHITE][PAWN] >> fromSq) & 1))
                moves.push_back(Move(fromSq, epSq, PAWN, EN_PASSANT));
        }
        // Pawn at epSq-7 captures NW (+7) to epSq: valid if epSq not on FILE_H
        if (!(epBit & FILE_H)) {
            int fromSq = epSq - 7;
            if (fromSq >= 0 && ((board.pieces[WHITE][PAWN] >> fromSq) & 1))
                moves.push_back(Move(fromSq, epSq, PAWN, EN_PASSANT));
        }
    } else {
        // Pawn at epSq+9 captures SW (-9) to epSq: valid if epSq not on FILE_H
        if (!(epBit & FILE_H)) {
            int fromSq = epSq + 9;
            if (fromSq < 64 && ((board.pieces[BLACK][PAWN] >> fromSq) & 1))
                moves.push_back(Move(fromSq, epSq, PAWN, EN_PASSANT));
        }
        // Pawn at epSq+7 captures SE (-7) to epSq: valid if epSq not on FILE_A
        if (!(epBit & FILE_A)) {
            int fromSq = epSq + 7;
            if (fromSq < 64 && ((board.pieces[BLACK][PAWN] >> fromSq) & 1))
                moves.push_back(Move(fromSq, epSq, PAWN, EN_PASSANT));
        }
    }
    
    
}

// --- Free-function wrappers used by search.h ---
void generate_legal_moves(const Board& board, std::vector<Move>& moves) {
    MoveGenerator gen;
    gen.generatePseudoLegalMoves(board, moves);
    // search.h's make_move/unmake_move loop filters illegal moves lazily.
}

void generate_captures(const Board& board, std::vector<Move>& moves) {
    std::vector<Move> all_moves;
    MoveGenerator gen;
    gen.generatePseudoLegalMoves(board, all_moves);

    for (const Move& m : all_moves) {
        if (m.isCapture() || m.isPromotion()) {
            moves.push_back(m);
        }
    }
}

