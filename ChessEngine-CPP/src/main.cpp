#include <iostream>
#include <string>
#include <sstream>
#include "board.h"
#include "movegen.h"
#include "search.h"



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

            if(type == "startpos")
            {
                board.reset();
            }

            else if(type == "fen")
            {
                std::string fen;
                std::getline(iss>>std::ws,fen);
                board.setFromFEN(fen);
            }
        }

        else if (command == "go")
        {
            Move best = search_best_move(board, /*max_depth=*/6);

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

