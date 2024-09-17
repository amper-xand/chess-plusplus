#pragma once

#include "../../utils/utils.hpp"
#include "../game.hpp"

#include <span>
#include <vector>

namespace Game::Generators::Helpers {

    // Last index of captures is a bitboard for all captures
    void populate_from_bitboard(std::span<Move> target, bitboard moves,
                                bitboard captures, Board board, square origin);

    template <bitboard (*mgenerator)(bitboard blockers, square index),
              Pieces::Piece piece>
    std::vector<Move> moves_from_generator(Board board);

    // Definition

    template <bitboard (*mgenerator)(bitboard blockers, square index),
              Pieces::Piece piece>
    std::vector<Move> moves_from_generator(Board board) {

        bitboard pieces = board.pieces[piece] & board.colors[board.turn],
                 all = board.all_pieces(), blockers = all;

        uint8_t pieces_count = std::popcount(pieces);

        uint16_t total_avail_moves = 0;
        bitboard available_per_piece[pieces_count];
        square piece_positions[pieces_count];

        for (square index = 0, current_piece = 0; pieces != 0 && index < 64;
             ++index, pieces >>= 1) {

            if (Utils::last_bit(pieces)) {
                auto available_moves =
                    mgenerator(blockers, index) & ~board.colors[board.turn];

                piece_positions[current_piece] = index;

                available_per_piece[current_piece] = available_moves;
                total_avail_moves += std::popcount(available_moves);

                ++current_piece;
            }
        }

        std::vector<Move> moves(total_avail_moves);

        if (moves.size() == 0)
            return moves;

        std::fill(moves.begin(), moves.end(), Move{.piece_moved = piece});

        auto current_move = moves.begin();

        for (uint8_t current_piece = 0; current_piece < pieces_count;
             ++current_piece) {

            uint8_t size = std::popcount(available_per_piece[current_piece]);

            bitboard captures = available_per_piece[current_piece] & all;

            populate_from_bitboard(std::span(current_move, size),
                                   available_per_piece[current_piece], captures,
                                   board, piece_positions[current_piece]);

            std::advance(current_move, size);
        }

        return moves;
    }
} // namespace Game::Generators::Helpers
