#pragma once
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>
#include "board.h"
#include "move.h"
#include "movegen.h"
#include "eval.h"

constexpr int INF = 1000000;
constexpr int CHECKMATE_SCORE = 900000;
constexpr int MAX_PLY = 64;

// MVV-LVA Table: [Attacker][Victim]
constexpr int MVV_LVA[6][6] = {
    // Victim:  P     N     B     R     Q     K
    /* P */  { 105,  205,  305,  405,  505,    0 },
    /* N */  { 104,  204,  304,  404,  504,    0 },
    /* B */  { 103,  203,  303,  403,  503,    0 },
    /* R */  { 102,  202,  302,  402,  502,    0 },
    /* Q */  { 101,  201,  301,  401,  501,    0 },
    /* K */  { 100,  200,  300,  400,  500,    0 }
};

struct SearchStats {
    uint64_t nodes = 0;
};

// -------------------------------------------------------------------------
// Helper: Format a Move to UCI string using board.squareName()
// -------------------------------------------------------------------------
inline std::string format_uci(const Board& board, const Move& m) {
    std::string str = board.squareName(m.getFrom()) + board.squareName(m.getTo());
    if (m.getMoveType() == PROMOTION) {
        constexpr char promo_chars[6] = { 'p', 'n', 'b', 'r', 'q', 'k' };
        str += promo_chars[m.getPromoteTo()];
    }
    return str;
}

// -------------------------------------------------------------------------
// Move Ordering
// -------------------------------------------------------------------------
inline int score_move(const Board& board, const Move& move) {
    MoveType type = move.getMoveType();

    if (type == CAPTURE || type == EN_PASSANT) {
        int attacker = move.getPieceType();
        int victim = (type == EN_PASSANT) ? PAWN : board.piece_at(move.getTo());

        if (victim >= 0 && attacker >= 0) {
            return 10000 + MVV_LVA[attacker][victim];
        }
        return 10000;
    }

    if (type == PROMOTION) {
        return 9000;
    }

    return 0; // Quiet moves
}

inline void order_moves(const Board& board, std::vector<Move>& moves) {
    std::vector<std::pair<int, Move>> scored_moves;
    scored_moves.reserve(moves.size());

    for (const auto& m : moves) {
        scored_moves.emplace_back(score_move(board, m), m);
    }

    std::sort(scored_moves.begin(), scored_moves.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    for (size_t i = 0; i < moves.size(); ++i) {
        moves[i] = scored_moves[i].second;
    }
}

// -------------------------------------------------------------------------
// Quiescence Search
// -------------------------------------------------------------------------
inline int quiescence(Board& board, int alpha, int beta, int ply, SearchStats& stats) {
    stats.nodes++;

    if (ply >= MAX_PLY) {
        return evaluate(board);
    }

    int stand_pat = evaluate(board);

    if (stand_pat >= beta) {
        return beta;
    }
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }

    std::vector<Move> capture_moves;
    generate_captures(board, capture_moves);
    order_moves(board, capture_moves);

    for (const Move& move : capture_moves) {
        UndoInfo undo;
        if (!board.make_move(move, undo)) {
            continue;
        }

        int score = -quiescence(board, -beta, -alpha, ply + 1, stats);
        board.unmake_move(move, undo);

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

// -------------------------------------------------------------------------
// Negamax Alpha-Beta Search
// -------------------------------------------------------------------------
inline int negamax(Board& board, int depth, int alpha, int beta, int ply, SearchStats& stats) {
    stats.nodes++;

    if (board.halfmoveClock >= 100 && ply > 0) {
        return 0; // 50-move rule
    }

    if (depth <= 0 || ply >= MAX_PLY) {
        return quiescence(board, alpha, beta, ply, stats);
    }

    bool in_check = board.is_in_check(board.turn);
    if (in_check) {
        depth++; // Check extension
    }

    std::vector<Move> moves;
    generate_legal_moves(board, moves);

    if (moves.empty()) {
        return in_check ? (-CHECKMATE_SCORE + ply) : 0;
    }

    order_moves(board, moves);

    int best_score = -INF;
    int legal_moves_played = 0;

    for (const Move& move : moves) {
        UndoInfo undo;
        if (!board.make_move(move, undo)) {
            continue;
        }

        legal_moves_played++;
        int score = -negamax(board, depth - 1, -beta, -alpha, ply + 1, stats);
        board.unmake_move(move, undo);

        if (score > best_score) {
            best_score = score;
        }

        if (score > alpha) {
            alpha = score;
        }

        if (alpha >= beta) {
            break; // Beta cutoff
        }
    }

    if (legal_moves_played == 0) {
        return in_check ? (-CHECKMATE_SCORE + ply) : 0;
    }

    return best_score;
}

// -------------------------------------------------------------------------
// Iterative Deepening Root Search
// -------------------------------------------------------------------------
inline Move search_best_move(Board& board, int max_depth) {
    SearchStats stats;

    std::vector<Move> root_moves;
    generate_legal_moves(board, root_moves);

    if (root_moves.empty()) {
        return Move(0, 0, PAWN, NORMAL);
    }

    Move best_move_overall = root_moves[0];
    auto start_time = std::chrono::steady_clock::now();

    for (int depth = 1; depth <= max_depth; ++depth) {
        int alpha = -INF;
        int beta = INF;
        int best_score_depth = -INF;
        Move best_move_depth = root_moves[0];

        order_moves(board, root_moves);

        for (const Move& move : root_moves) {
            UndoInfo undo;
            if (!board.make_move(move, undo)) {
                continue;
            }

            int score = -negamax(board, depth - 1, -beta, -alpha, 1, stats);
            board.unmake_move(move, undo);

            if (score > best_score_depth) {
                best_score_depth = score;
                best_move_depth = move;
            }

            if (score > alpha) {
                alpha = score;
            }
        }

        best_move_overall = best_move_depth;

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start_time
        ).count();
        uint64_t nps = elapsed > 0 ? (stats.nodes * 1000) / elapsed : 0;

        if (std::abs(best_score_depth) >= CHECKMATE_SCORE - MAX_PLY) {
            int mate_in = (CHECKMATE_SCORE - std::abs(best_score_depth) + 1) / 2;
            std::cout << "info depth " << depth
                      << " score mate " << (best_score_depth > 0 ? mate_in : -mate_in)
                      << " nodes " << stats.nodes
                      << " time " << elapsed
                      << " nps " << nps
                      << " pv " << format_uci(board, best_move_overall) << std::endl;
            break;
        } else {
            std::cout << "info depth " << depth
                      << " score cp " << best_score_depth
                      << " nodes " << stats.nodes
                      << " time " << elapsed
                      << " nps " << nps
                      << " pv " << format_uci(board, best_move_overall) << std::endl;
        }
    }

    return best_move_overall;
}