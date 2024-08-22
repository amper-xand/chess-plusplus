#include "generators.hpp"

#include <bit>
#include <cstdint>

namespace Game::Generators {
    std::vector<Move> gen_pawn_moves(Game::Board board) {
        bitboard blockers = board.white | board.white;

        bitboard pawns = board.pawns & board.colors[board.turn];

        bitboard single_advance = pawns;

        if (board.turn == Colors::WHITE) {
            single_advance <<= 8;
        } else {
            single_advance >>= 8;
        }

        // remove blocked pawns
        single_advance &= ~blockers;

        bitboard double_advance = single_advance;

        if (board.turn == Colors::WHITE) {
            double_advance <<= 8;
        } else {
            double_advance >>= 8;
        }

        // remove blocked pawns
        double_advance &= ~blockers;

        // return pawns to their original positions
        if (board.turn == Colors::WHITE) {
            single_advance >>= 8;
            double_advance >>= 16;
        } else {
            single_advance <<= 8;
            double_advance <<= 16;
        }

        std::vector<Move> moves(std::popcount(single_advance) +
                                std::popcount(double_advance));

        if (moves.size() == 0) {
            return moves;
        }

        uint8_t m = 0;

        for (; single_advance != 0; ++m, single_advance &= single_advance - 1) {
            square index = std::countr_zero(single_advance);

            auto &move = moves.at(m);
            move.moved = Pieces::PAWNS;
            move.from = index;

            if (board.turn == Colors::WHITE)
                move.to = index + 8;
            else
                move.to = index - 8;
        }

        for (; double_advance != 0; ++m, double_advance &= double_advance - 1) {
            square index = std::countr_zero(double_advance);

            auto &move = moves.at(m);
            move.moved = Pieces::PAWNS;
            move.from = index;

            if (board.turn == Colors::WHITE)
                move.to = index + 16;
            else
                move.to = index - 16;
        }

        return moves;
    }

} // namespace Game::Generators
