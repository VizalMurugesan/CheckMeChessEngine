#pragma once
#include <cstdint>
#include <string>
#include <cctype>

enum Color {WHITE = 0, BLACK = 1};
enum PieceType {PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5};


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
    void setFromFEN(const std::string& fen){

        clearTheBoard();
        int rank = 7;
        int file = 0;
        size_t index = 0;

        while (index < fen.length() && fen[index] != ' ')
        {
            
            char c = fen[index];

            if (c >= '0' && c <= '9') {
                file += (c - '0'); // Converts character '8' to integer 8
                index++;
            }

            else{

                if(c == '/'){
                    rank--;
                    file = 0;
                }

                int pieceNameIndex = getPieceName(c);
                int pieceColorIndex = (c >= 'A' && c <= 'Z') ? 0 : 1;
                int pos = rank*8 + file;

                pieces[pieceColorIndex][pieceNameIndex]|= (1ULL <<pos);

                index++;
            }
            

        }

        if (index < fen.length()) index++;
        // --- FIELD 2: ACTIVE COLOR ---
        if (index < fen.length() && fen[index] != ' ') {
            turn = (fen[index] == 'w') ? WHITE : BLACK;
            index++;
        }

        // Advance past space to reach Field 3: Castling Rights
        if (index < fen.length() && fen[index] == ' ') index++;

        // --- FIELD 3: CASTLING RIGHTS ---
        castlingRights = 0; // Clear it out to rebuild it from the string flags
        while (index < fen.length() && fen[index] != ' ') {
            char c = fen[index];
            if (c == '-') { 
                index++; 
                break; 
            }
            if (c == 'K') castlingRights |= (1 << 0); // Bit 0: White King-side
            if (c == 'Q') castlingRights |= (1 << 1); // Bit 1: White Queen-side
            if (c == 'k') castlingRights |= (1 << 2); // Bit 2: Black King-side
            if (c == 'q') castlingRights |= (1 << 3); // Bit 3: Black Queen-side
            index++;
        }

        // Advance past space to reach Field 4: En Passant Target
        if (index < fen.length() && fen[index] == ' ') index++;

        // --- FIELD 4: EN PASSANT TARGET SQUARE ---
        if (index < fen.length() && fen[index] != ' ') {
            if (fen[index] == '-') {
                enPassantSquare = -1;
                index++;
            } else if (index + 1 < fen.length()) {
                // FEN coordinates are text (e.g., "e3"). Convert to 0-63 index.
                int epFile = fen[index] - 'a';       // 'e' - 'a' = 4
                int epRank = fen[index + 1] - '1';   // '3' - '1' = 2
                
                enPassantSquare = epRank * 8 + epFile; // 2 * 8 + 4 = 20
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
};