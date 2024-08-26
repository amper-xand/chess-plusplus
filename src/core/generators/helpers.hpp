#pragma once

#include "../../utils/utils.hpp"
#include "../game.hpp"

#include <span>
#include <vector>

namespace Game::Generators::Helpers {

    // Last index of captures is a bitboard for all captures
    void populate_from_bitboard(std::span<Move> target, bitboard bitboard,
                                ::Game::bitboard captures[7], square origin);

    template <bitboard (*mgenerator)(bitboard blockers, square index),
              Pieces::Piece piece>
    std::vector<Move> moves_from_generator(Game::Board board) {
        auto pieces = board.pieces[piece] & board.colors[board.turn];

        bitboard blockers = board.white | board.black;

        uint8_t pieces_count = std::popcount(pieces);

        uint16_t available_moves = 0;
        bitboard available_per_piece[pieces_count];
        square piece_positions[pieces_count];

        uint8_t current_piece = 0;

        for (square index = 0; pieces != 0 & index < 64;
             ++index, pieces >>= 1) {

            if (Utils::last_bit(pieces)) {
                available_per_piece[current_piece] =
                    mgenerator(blockers, index) & ~board.colors[board.turn];

                piece_positions[current_piece] = index;

                available_moves +=
                    std::popcount(available_per_piece[current_piece]);

                ++current_piece;
            }
        }

        std::vector<Move> moves(available_moves);

        if (moves.size() == 0)
            return moves;

        std::fill(moves.begin(), moves.end(), Move{.piece_moved = piece});

        auto current_move = moves.begin();

        for (current_piece = 0; current_piece < pieces_count; ++current_piece) {
            uint8_t size = std::popcount(available_per_piece[current_piece]);

            bitboard captures_by_type[7];
            captures_by_type[6] = 0;

            for (auto type : Pieces::AllPieces) {
                captures_by_type[type] =
                    available_per_piece[current_piece] & board.pieces[type];
                captures_by_type[6] |= captures_by_type[type];
            }

            populate_from_bitboard(std::span(current_move, size),
                                   available_per_piece[current_piece],
                                   captures_by_type,
                                   piece_positions[current_piece]);

            std::advance(current_move, size);
        }

        return moves;
    }

}
