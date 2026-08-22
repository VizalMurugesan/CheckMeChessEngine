#pragma once
#include <cstdint>
#include <string>
#include <cctype>

enum Color {WHITE = 0, BLACK = 1};
enum PieceType {PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5};
struct Move; 
struct UndoInfo {
    int      capturedPieceType   = -1;   // -1 if the move captured nothing
    int8_t   prevEnPassantSquare = -1;
    uint8_t  prevCastlingRights  = 0;
    int      prevHalfmoveClock   = 0;
    int      prevFullmoveNumber  = 1;
};




struct Board {

    uint64_t pieces[2][6];
    Color turn;
    uint8_t castlingRights;   // bit 0: WK, bit 1: WQ, bit 2: BK, bit 3: BQ
    int8_t  enPassantSquare;  // -1 if none, otherwise target square index (0-63)
    // 5. Game State Clocks (New variables needed for full FEN compliance)
    int halfmoveClock;   // Tracks 50-move draw rule
    int fullmoveNumber;  // Increments after every Black move
    
    Board(){
       reset();
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

    void clearTheBoard() {
        for (int color = 0; color < 2; ++color) {
            for (int piece = 0; piece < 6; ++piece) {
                pieces[color][piece] = 0ULL;
            }
        }
        turn = WHITE;
        castlingRights = 0;
        enPassantSquare = -1;
    }

    void reset() {
        clearTheBoard();

        // White pieces
        pieces[WHITE][PAWN]   = 0x000000000000FF00;
        pieces[WHITE][ROOK]   = 0x0000000000000081;
        pieces[WHITE][KNIGHT] = 0x0000000000000042;
        pieces[WHITE][BISHOP] = 0x0000000000000024;
        pieces[WHITE][QUEEN]  = 0x0000000000000008;
        pieces[WHITE][KING]   = 0x0000000000000010;

        // Black pieces
        pieces[BLACK][PAWN]   = 0x00FF000000000000;
        pieces[BLACK][ROOK]   = 0x8100000000000000;
        pieces[BLACK][KNIGHT] = 0x4200000000000000;
        pieces[BLACK][BISHOP] = 0x2400000000000000;
        pieces[BLACK][QUEEN]  = 0x0800000000000000;
        pieces[BLACK][KING]   = 0x1000000000000000;

        turn = WHITE;
        castlingRights = 0x0F;
        enPassantSquare = -1;
        halfmoveClock = 0;
        fullmoveNumber = 1;
    }

//FEN PARSING
    #pragma region 
    
    void setFromFEN(const std::string& fen) {
        clearTheBoard();
        int rank = 7;
        int file = 0;
        size_t index = 0;

        // --- FIELD 1: PIECE PLACEMENT ---
        while (index < fen.length() && fen[index] != ' ') {
            char c = fen[index];

            if (c >= '0' && c <= '9') {
                file += (c - '0');
                index++;
            }
            else if (c == '/') {
                rank--;
                file = 0;
                index++;
            }
            else {
                int pieceNameIndex = getPieceName(c);
                int pieceColorIndex = (c >= 'A' && c <= 'Z') ? WHITE : BLACK;

                if (pieceNameIndex >= 0 && rank >= 0 && rank < 8 && file >= 0 && file < 8) {
                    int pos = rank * 8 + file;
                    pieces[pieceColorIndex][pieceNameIndex] |= (1ULL << pos);
                }

                file++;
                index++;
            }
        }

        // Advance to Field 2
        if (index < fen.length() && fen[index] == ' ') index++;

        // --- FIELD 2: ACTIVE COLOR ---
        if (index < fen.length() && fen[index] != ' ') {
            turn = (fen[index] == 'w') ? WHITE : BLACK;
            index++;
        }

        // Advance to Field 3
        if (index < fen.length() && fen[index] == ' ') index++;

        // --- FIELD 3: CASTLING RIGHTS ---
        castlingRights = 0;
        while (index < fen.length() && fen[index] != ' ') {
            char c = fen[index];
            if (c == '-') {
                index++;
                break;
            }
            if (c == 'K') castlingRights |= (1 << 0);
            if (c == 'Q') castlingRights |= (1 << 1);
            if (c == 'k') castlingRights |= (1 << 2);
            if (c == 'q') castlingRights |= (1 << 3);
            index++;
        }

        // Advance to Field 4
        if (index < fen.length() && fen[index] == ' ') index++;

        // --- FIELD 4: EN PASSANT TARGET SQUARE ---
        if (index < fen.length() && fen[index] != ' ') {
            if (fen[index] == '-') {
                enPassantSquare = -1;
                index++;
            } else if (index + 1 < fen.length()) {
                int epFile = fen[index] - 'a';
                int epRank = fen[index + 1] - '1';
                enPassantSquare = epRank * 8 + epFile;
                index += 2;
            }
        }
    }

    int getPieceName( char piece){

        piece = std::tolower(static_cast<unsigned char> (piece));
        switch (piece)
        {
        case 'r':
            return 3;
        
        case 'n':
            return 1;   
        
        case 'b':
            return 2;
            
        case 'k':
            return 5;
            
        case 'q':
            return 4;
        
        case 'p':
            return 0;
        
        default:
            return 6;
            
        }
    }

    

    #pragma endregion

    std :: string squareName(int sq) const{
        char file = 'a' + (sq % 8);
        char rank = '1' + (sq / 8);
        return std::string(1, file) + std::string(1, rank);
    }

    // --- PIECE QUERY HELPERS ---

    // Returns the PieceType on a square (0-5), or -1 if empty
    inline int piece_at(int sq) const {
        uint64_t mask = 1ULL << sq;
        for (int p = 0; p < 6; ++p) {
            if ((pieces[WHITE][p] | pieces[BLACK][p]) & mask) {
                return p;
            }
        }
        return -1; // EMPTY
    }

      // Make/unmake a move on this board (defined in board.cpp).
    // make_move returns false (and auto-reverts) if the move leaves your own king in check.
    bool make_move(const Move& move, UndoInfo& undo);
    void unmake_move(const Move& move, const UndoInfo& undo);
    bool is_in_check(Color side) const;

    // Returns the Color on a square, or -1 if empty
    inline int color_at(int sq) const {
        uint64_t mask = 1ULL << sq;
        if (whitePieces() & mask) return WHITE;
        if (blackPieces() & mask) return BLACK;
        return -1; // EMPTY
    }

    // Check if a specific square is occupied by either side
    inline bool is_occupied(int sq) const {
        return (occupied() & (1ULL << sq)) != 0;
    }

    
};