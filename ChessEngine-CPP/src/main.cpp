#include <iostream>
#include <string>
#include <sstream>
#include "board.h"
#include "movegen.h"
#include "search.h"

// Parse a UCI move string (e.g. "e2e4", "e7e8q") and apply it to the board.
static void apply_uci_move(Board& board, const std::string& moveStr) {
    if (moveStr.size() < 4) return;

    int from = ((moveStr[1] - '1') * 8) + (moveStr[0] - 'a');
    int to   = ((moveStr[3] - '1') * 8) + (moveStr[2] - 'a');

    PieceType promoPiece = QUEEN;
    if (moveStr.size() >= 5) {
        switch (moveStr[4]) {
            case 'r': promoPiece = ROOK;   break;
            case 'b': promoPiece = BISHOP; break;
            case 'n': promoPiece = KNIGHT; break;
            default:  promoPiece = QUEEN;  break;
        }
    }

    std::vector<Move> moves;
    generate_legal_moves(board, moves);

    for (const Move& m : moves) {
        if ((int)m.getFrom() != from || (int)m.getTo() != to) continue;
        if (m.getMoveType() == PROMOTION && m.getPromoteTo() != promoPiece) continue;
        UndoInfo undo;
        board.make_move(m, undo);
        return;
    }
}

int main(){

    std::string line;
    Board board;
    MoveGenerator gen;

    while (std::getline(std::cin,line)){
        std::string command;
        std::istringstream iss(line);
        iss >> command;

        if (command ==  "uci")
        {
            std::cout << "id name CheckMe" << std::endl;
            std::cout << "id author Vizal" << std::endl;
            std::cout << "uciok" << std::endl;
        }

        else if (command =="quit")
        {
            break;
        }

        else if (command == "isready")
        {
            std::cout << "readyok" << std::endl;
        }

        else if (command =="position")
        {
            std::string type;
            iss >> type;

            if (type == "startpos")
            {
                board.reset();
                std::string token;
                if (iss >> token && token == "moves") {
                    std::string mv;
                    while (iss >> mv) apply_uci_move(board, mv);
                }
            }

            else if (type == "fen")
            {
                // Read everything after "fen", then split on " moves "
                std::string rest;
                std::getline(iss >> std::ws, rest);

                size_t movesPos = rest.find(" moves ");
                std::string fen = (movesPos != std::string::npos) ? rest.substr(0, movesPos) : rest;
                board.setFromFEN(fen);

                if (movesPos != std::string::npos) {
                    std::istringstream moveStream(rest.substr(movesPos + 7));
                    std::string mv;
                    while (moveStream >> mv) apply_uci_move(board, mv);
                }
            }
        }

        else if (command == "go")
        {
            Move best = search_best_move(board, /*max_depth=*/5);

            if (!best.isNull())
            {
                std::string moveStr = board.squareName(best.getFrom()) + board.squareName(best.getTo());

                if (best.getMoveType() == PROMOTION) {
                    switch (best.getPromoteTo()) {
                        case QUEEN:  moveStr += 'q'; break;
                        case ROOK:   moveStr += 'r'; break;
                        case BISHOP: moveStr += 'b'; break;
                        case KNIGHT: moveStr += 'n'; break;
                        default:     moveStr += 'q'; break;
                    }
                }

                std::cout << "bestmove " << moveStr << std::endl;
            }
            else
            {
                std::cout << "bestmove none" << std::endl;
            }
        }

        
        
    }
}

