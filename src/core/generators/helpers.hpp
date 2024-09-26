#pragma once

#include "../representation/representation.hpp"
#include "generators.hpp"

#include <algorithm>
#include <cstdint>
#include <span>

namespace Game::Generators::Helpers {

    void populate_from_bitboard(std::span<Move> target, bitboard moves,
                                bitboard captures, Board board, square origin);

    inline void scan(bitboard bitboard, const auto& proccess) {
        for (square index = 0; bitboard != 0 && index < 64;
             ++index, bitboard >>= 1) {

            if (bitboard.last_bit()) {
                proccess(bitboard, index);
            }
        }
    }

    template <auto get_moves, Pieces::Piece piece>
    MoveGenerator& moves_from_generator(MoveGenerator& generator) {
        auto& board = generator.board;

        bitboard pieces = board.allied(piece), blockers = board.all_pieces(),
                 noncaptures = board.allies();

        // Remove pinned pieces
        pieces &= ~generator.pinned;

        uint8_t piece_count = pieces.popcount();

        uint16_t total_avail_moves = 0;
        bitboard available_per_piece[piece_count];
        square piece_positions[piece_count];

        uint8_t current_piece = 0;
        auto move_scanner = [&](bitboard pieces, square index) {
            bitboard available_moves = get_moves(blockers, index) & ~noncaptures;

            piece_positions[current_piece] = index;

            available_per_piece[current_piece] = available_moves;
            total_avail_moves += available_moves.popcount();

            ++current_piece;
        };

        scan(pieces, move_scanner);

        if (total_avail_moves == 0)
            return generator;

        for (uint8_t current_piece = 0; current_piece < piece_count;
             ++current_piece) {

            uint8_t move_count = available_per_piece[current_piece].popcount();

            bitboard captures =
                available_per_piece[current_piece] & board.enemies();

            std::span<Move> moves = generator.next_n(move_count);

            std::fill(moves.begin(), moves.end(),
                      Move{.piece = {.moved = piece}});

            populate_from_bitboard(moves, available_per_piece[current_piece],
                                   captures, board,
                                   piece_positions[current_piece]);
        }

        return generator;
    }
} // namespace Game::Generators::Helpers
