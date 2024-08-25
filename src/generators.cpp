#include "generators.hpp"
#include "game.hpp"
#include "magic.hpp"
#include "utils.hpp"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <span>
#include <stdexcept>
#include <vector>

namespace Game::Generators {
    std::vector<Move> gen_pawn_moves(Game::Board board) {
        bitboard blockers = board.all();

        bitboard pawns = board.pawns & board.colors[board.turn];

        bitboard advances = pawns;

        if (board.turn == Colors::WHITE) {
            advances <<= 8;
        } else {
            advances >>= 8;
        }

        // remove blocked pawns
        advances &= ~blockers;

        bitboard double_advances =
            advances & (0xFF0000 ? board.turn == Colors::WHITE
                                 : Utils::flip_vertically(0xFF0000));

        if (board.turn == Colors::WHITE) {
            double_advances <<= 8;
        } else {
            double_advances >>= 8;
        }

        // remove blocked pawns
        double_advances &= ~blockers;

        // return pawns to their original positions
        if (board.turn == Colors::WHITE) {
            advances >>= 8;
            double_advances >>= 16;
        } else {
            advances <<= 8;
            double_advances <<= 16;
        }

        std::vector<Move> moves(std::popcount(advances) +
                                std::popcount(double_advances));

        if (moves.size() == 0) {
            return moves;
        }

        auto move = moves.begin();

        auto from_advances = [&](bitboard &advance, int offset) {
            for (square index = 0; index < 64; ++index, advance >>= 1) {
                if (Utils::last_bit(advance)) {
                    move->moved = Pieces::PAWNS;
                    move->from = index;

                    if (board.turn == Colors::WHITE)
                        move->to = index + offset;
                    else
                        move->to = index - offset;

                    if (move == moves.end() && advance != 1) {
                        throw std::out_of_range(
                            "Moves generated exceeded allocated space");
                    }

                    std::advance(move, 1);
                }
            }
        };

        from_advances(advances, 8);
        from_advances(double_advances, 16);

        return moves;
    }

    void populate_from_bitboard(std::span<Move> target, bitboard bitboard,
                                square origin) {
        auto next_move = target.begin();

        for (square index = 0; index < 64; ++index, bitboard >>= 1) {
            if (Utils::last_bit(bitboard)) {
                next_move->from = origin;
                next_move->to = index;

                if (next_move == target.end() && bitboard != 1) {
                    throw std::out_of_range("Bitboard has more elements than "
                                            "the provided span can hold");
                }

                std::advance(next_move, 1);
            }
        }
    }

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

        for (square index = 0; index < 64; ++index, pieces >>= 1) {
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

        std::fill(moves.begin(), moves.end(), Move{.moved = piece});

        auto current_move = moves.begin();

        for (current_piece = 0; current_piece < pieces_count; ++current_piece) {
            uint8_t size = std::popcount(available_per_piece[current_piece]);

            populate_from_bitboard(std::span(current_move, size),
                                   available_per_piece[current_piece],
                                   piece_positions[current_piece]);

            std::advance(current_move, size);
        }

        return moves;
    }

    std::vector<Move> gen_rooks_moves(Game::Board board) {
        return moves_from_generator<Magic::Rooks::get_avail_moves,
                                    Pieces::ROOKS>(board);
    }

    std::vector<Move> gen_bishops_moves(Game::Board board) {
        return moves_from_generator<Magic::Bishops::get_avail_moves,
                                    Pieces::BISHOPS>(board);
    }

    std::vector<Move> gen_queens_moves(Game::Board board) {
        return moves_from_generator<[](bitboard blockers, square index) {
            return Magic::Rooks::get_avail_moves(blockers, index) |
                   Magic::Bishops::get_avail_moves(blockers, index);
        },
                                    Pieces::QUEENS>(board);
    }

} // namespace Game::Generators
