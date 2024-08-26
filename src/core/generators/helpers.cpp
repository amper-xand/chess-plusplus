#include "helpers.hpp"

void Game::Generators::Helpers::populate_from_bitboard(
    std::span<Move> target, ::Game::bitboard bitboard,
    ::Game::bitboard captures[7], ::Game::square origin) {
    auto next_move = target.begin();

    for (square index = 0; index < 64; ++index, bitboard >>= 1) {
        if (Utils::last_bit(bitboard)) {
            next_move->from = origin;
            next_move->to = index;

            // clang-format off
                if (Utils::last_bit(captures[6])) for (auto piece : Pieces::AllPieces) 
                    if (Utils::last_bit(captures[piece])) {
                        next_move->piece_captured = piece;
                        break;
                    }
            // clang-format on

            if (next_move == target.end() && bitboard != 1) {
                throw std::out_of_range("Bitboard has more elements than "
                                        "the provided span can hold");
            }

            captures[6] >>= 1;
            for (auto piece : Pieces::AllPieces)
                captures[piece] >>= 1;

            std::advance(next_move, 1);
        }
    }
}
