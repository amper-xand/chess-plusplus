#pragma once

#include "../../utils/utils.hpp"
#include "../game.hpp"
#include "generators.hpp"

#include <algorithm>
#include <cstdint>
#include <span>
#include <tuple>

namespace Game::Generators::Helpers {

    void populate_from_bitboard(std::span<Move> target, bitboard moves,
                                bitboard captures, Board board, square origin);

    inline void scan(bitboard bitboard, const auto& proccess) {
        for (square index = 0; bitboard != 0 && index < 64;
             ++index, bitboard >>= 1) {

            if (Utils::last_bit(bitboard)) {
                proccess(bitboard, index);
            }
        }
    }

    template <uint8_t piece_count, auto get_moves>
    inline std::tuple<uint16_t, bitboard[piece_count], square[piece_count]>
    scan_moves(bitboard pieces, bitboard blockers, bitboard noncaptures) {

        uint16_t total_avail_moves = 0;
        bitboard available_per_piece[piece_count];
        square piece_positions[piece_count];

        uint8_t current_piece = 0;
        auto move_scanner = [&](bitboard pieces, square index) {
            auto available_moves = get_moves(blockers, index) & ~noncaptures;

            piece_positions[current_piece] = index;

            available_per_piece[current_piece] = available_moves;
            total_avail_moves += std::popcount(available_moves);

            ++current_piece;
        };

        scan(pieces, move_scanner);

        return {total_avail_moves, available_per_piece, piece_positions};
    }

    template <auto get_moves, Pieces::Piece piece>
    MoveGenerator& moves_from_generator(MoveGenerator& generator) {
        auto& board = generator.board;

        bitboard pieces = board.allied(piece), blockers = board.all_pieces(),
                 noncaptures = board.allies();

        // Remove pinned pieces
        pieces &= ~generator.pinned;

        uint8_t pieces_count = std::popcount(pieces);

        auto [total_avail_moves, available_per_piece, piece_positions] =
            scan_moves<pieces_count, get_moves>(pieces, blockers, noncaptures);

        if (total_avail_moves == 0)
            return generator;

        for (uint8_t current_piece = 0; current_piece < pieces_count;
             ++current_piece) {

            uint8_t move_count =
                std::popcount(available_per_piece[current_piece]);

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
