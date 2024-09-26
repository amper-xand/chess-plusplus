#include "helpers.hpp"

#include <stdexcept>

namespace Game::Generators::Helpers {
    void populate_from_bitboard(std::span<Move> target, bitboard moves,
                                bitboard captures, Board board, square origin) {

        auto next_move = target.begin();

        for (square index = 0; index < 64;
             ++index, moves >>= 1, captures >>= 1) {

            if (moves.last_bit()) {
                next_move->from = origin;
                next_move->to = index;

                if (captures.last_bit())
                    next_move->piece.captured = board.piece_at(index);

                if (next_move == target.end() && moves != 1) {
                    throw std::out_of_range("Bitboard has more elements than "
                                            "the provided span can hold");
                }

                std::advance(next_move, 1);
            }
        }
    }

    
} // namespace Game::Generators::Helpers
