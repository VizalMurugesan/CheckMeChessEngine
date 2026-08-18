#include "board.h"
#include "move.h"
#include "movegen.h"

bool Board::is_in_check(Color side) const {
    MoveGenerator gen;
    return gen.isInCheck(*this, side);
}

bool Board::make_move(const Move& move, UndoInfo& undo) {
    int       from = static_cast<int>(move.getFrom());
    int       to   = static_cast<int>(move.getTo());
    PieceType pt   = move.getPieceType();
    MoveType  mt   = move.getMoveType();
    Color     side  = turn;
    Color     enemy = (side == WHITE) ? BLACK : WHITE;

    // Save everything needed to undo
    undo.prevCastlingRights  = castlingRights;
    undo.prevEnPassantSquare = enPassantSquare;
    undo.prevHalfmoveClock   = halfmoveClock;
    undo.prevFullmoveNumber  = fullmoveNumber;
    undo.capturedPieceType   = -1;

    // 50-move rule clock
    if (pt == PAWN || mt == CAPTURE || mt == EN_PASSANT) {
        halfmoveClock = 0;
    } else {
        halfmoveClock++;
    }

    // Lift the moving piece
    pieces[side][pt] &= ~(1ULL << from);

    // Regular capture: find & remove the captured piece
    if (mt == CAPTURE) {
        for (int p = 0; p < 6; ++p) {
            if (pieces[enemy][p] & (1ULL << to)) {
                undo.capturedPieceType = p;
                pieces[enemy][p] &= ~(1ULL << to);
                break;
            }
        }
    }

    // Place piece (or promoted piece) on destination
    if (mt == PROMOTION) {
        pieces[side][move.getPromoteTo()] |= (1ULL << to);
    } else {
        pieces[side][pt] |= (1ULL << to);
    }

    // En passant capture
    if (mt == EN_PASSANT) {
        int capSq = (side == WHITE) ? (to - 8) : (to + 8);
        pieces[enemy][PAWN] &= ~(1ULL << capSq);
        undo.capturedPieceType = PAWN;
    }

    // Castling: also move the rook
    if (mt == CASTLING) {
        if      (to == 6)  { pieces[WHITE][ROOK] &= ~(1ULL<<7);  pieces[WHITE][ROOK] |= (1ULL<<5);  }
        else if (to == 2)  { pieces[WHITE][ROOK] &= ~(1ULL<<0);  pieces[WHITE][ROOK] |= (1ULL<<3);  }
        else if (to == 62) { pieces[BLACK][ROOK] &= ~(1ULL<<63); pieces[BLACK][ROOK] |= (1ULL<<61); }
        else if (to == 58) { pieces[BLACK][ROOK] &= ~(1ULL<<56); pieces[BLACK][ROOK] |= (1ULL<<59); }
    }

    // Update castling rights if king or rook moved / was captured
    if (pt == KING) {
        castlingRights &= (side == WHITE) ? ~0x3 : ~0xC;
    }
    if (from == 0  || to == 0)  castlingRights &= ~0x2; // a1 rook (WQ)
    if (from == 7  || to == 7)  castlingRights &= ~0x1; // h1 rook (WK)
    if (from == 56 || to == 56) castlingRights &= ~0x8; // a8 rook (BQ)
    if (from == 63 || to == 63) castlingRights &= ~0x4; // h8 rook (BK)

    // Update en passant target square
    enPassantSquare = -1;
    if (pt == PAWN && (to - from == 16 || from - to == 16)) {
        enPassantSquare = (from + to) / 2;
    }

    if (side == BLACK) fullmoveNumber++;

    turn = enemy;

    // Legality check: did this leave the mover's own king in check?
    if (is_in_check(side)) {
        unmake_move(move, undo);
        return false;
    }

    return true;
}

void Board::unmake_move(const Move& move, const UndoInfo& undo) {
    int       from = static_cast<int>(move.getFrom());
    int       to   = static_cast<int>(move.getTo());
    PieceType pt   = move.getPieceType();
    MoveType  mt   = move.getMoveType();
    Color     side  = (turn == WHITE) ? BLACK : WHITE; // side that made the move
    Color     enemy = turn;

    turn = side;

    if (mt == PROMOTION) {
        pieces[side][move.getPromoteTo()] &= ~(1ULL << to);
    } else {
        pieces[side][pt] &= ~(1ULL << to);
    }
    pieces[side][pt] |= (1ULL << from);

    if (mt == CAPTURE && undo.capturedPieceType >= 0) {
        pieces[enemy][undo.capturedPieceType] |= (1ULL << to);
    }

    if (mt == EN_PASSANT) {
        int capSq = (side == WHITE) ? (to - 8) : (to + 8);
        pieces[enemy][PAWN] |= (1ULL << capSq);
    }

    if (mt == CASTLING) {
        if      (to == 6)  { pieces[WHITE][ROOK] &= ~(1ULL<<5);  pieces[WHITE][ROOK] |= (1ULL<<7);  }
        else if (to == 2)  { pieces[WHITE][ROOK] &= ~(1ULL<<3);  pieces[WHITE][ROOK] |= (1ULL<<0);  }
        else if (to == 62) { pieces[BLACK][ROOK] &= ~(1ULL<<61); pieces[BLACK][ROOK] |= (1ULL<<63); }
        else if (to == 58) { pieces[BLACK][ROOK] &= ~(1ULL<<59); pieces[BLACK][ROOK] |= (1ULL<<56); }
    }

    castlingRights  = undo.prevCastlingRights;
    enPassantSquare = undo.prevEnPassantSquare;
    halfmoveClock   = undo.prevHalfmoveClock;
    fullmoveNumber  = undo.prevFullmoveNumber;
}