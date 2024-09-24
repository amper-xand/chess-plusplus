#pragma once

#include "../../utils/utils.hpp"
#include "../game.hpp"
#include "generators.hpp"

#include <algorithm>
#include <span>

namespace Game::Generators::Helpers {

    void populate_from_bitboard(std::span<Move> target, bitboard moves,
                                bitboard captures, Board board, square origin);

    template <bitboard (*mgenerator)(bitboard blockers, square index),
              Pieces::Piece piece>
    MoveGenerator& moves_from_generator(MoveGenerator& generator);

    // Definition

    template <bitboard (*mgenerator)(bitboard blockers, square index),
              Pieces::Piece piece>
    MoveGenerator& moves_from_generator(MoveGenerator& generator) {
        auto& board = generator.board;

        bitboard pieces = board.allied(piece), all = board.all_pieces(),
                 blockers = all;

        // Remove pinned pieces
        pieces &= ~generator.pinned;

        uint8_t pieces_count = std::popcount(pieces);

        uint16_t total_avail_moves = 0;
        bitboard available_per_piece[pieces_count];
        square piece_positions[pieces_count];

        for (square index = 0, current_piece = 0; pieces != 0 && index < 64;
             ++index, pieces >>= 1) {

            if (Utils::last_bit(pieces)) {
                auto available_moves =
                    mgenerator(blockers, index) & ~board.allies();

                piece_positions[current_piece] = index;

                available_per_piece[current_piece] = available_moves;
                total_avail_moves += std::popcount(available_moves);

                ++current_piece;
            }
        }

        if (total_avail_moves == 0)
            return generator;

        for (uint8_t current_piece = 0; current_piece < pieces_count;
             ++current_piece) {

            uint8_t move_count =
                std::popcount(available_per_piece[current_piece]);

            bitboard captures =
                available_per_piece[current_piece] & board.enemies();

            std::span<Move> moves = generator.next_n(move_count);

            std::fill(moves.begin(), moves.end(), Move{.piece = {piece}});

            populate_from_bitboard(moves, available_per_piece[current_piece],
                                   captures, board,
                                   piece_positions[current_piece]);
        }

        return generator;
    }
} // namespace Game::Generators::Helpers
