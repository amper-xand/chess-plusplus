#include "generators.hpp"

#include "../../utils/utils.hpp"
#include "../game.hpp"
#include "magic.hpp"
#include "helpers.hpp"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <iterator>
#include <span>
#include <stdexcept>
#include <strings.h>
#include <vector>

namespace Game::Generators::Knights {
    bitboard available_moves[64];

    bitboard gen_knight_moves(square index) {
        bitboard moves = 0;
        square col = Utils::column(index);

        if (col >= 1) {
            moves |= 1 << (index - 1 - 16);
            moves |= 1 << (index - 1 + 16);
        }

        if (col >= 2) {
            moves |= 1 << (index - 2 - 8);
            moves |= 1 << (index - 2 + 8);
        }

        if (col <= 6) {
            moves |= 1 << (index + 1 - 8);
            moves |= 1 << (index + 1 + 8);
        }

        if (col <= 5) {
            moves |= 1 << (index + 1 - 16);
            moves |= 1 << (index + 1 + 16);
        }

        return moves;
    }

    void initialize_table() {
        for (square index = 0; index < 64; ++index) {
            available_moves[index] = gen_knight_moves(index);
        }
    }
} // namespace Game::Generators::Knights

namespace Game::Generators {
    void initialize_tables() {
        Magic::initialize_magic_tables();
        Knights::initialize_table();
    }

    std::vector<Move> gen_pawn_moves(Game::Board board) {
        bitboard blockers = board.all_pieces();

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

        bitboard east_captures = pawns & ~0x0101010101010101;

        if (board.turn == Colors::WHITE) {
            east_captures <<= 7;
        } else {
            east_captures >>= 9;
        }

        east_captures &= board.colors[!board.turn];

        bitboard west_captures = pawns & ~0x8080808080808080;

        if (board.turn == Colors::WHITE) {
            west_captures <<= 9;
        } else {
            west_captures >>= 7;
        }

        west_captures &= board.colors[!board.turn];

        std::vector<Move> moves(
            std::popcount(advances) + std::popcount(double_advances) +
            std::popcount(east_captures) + std::popcount(west_captures));

        if (moves.size() == 0) {
            return moves;
        }

        auto move = moves.begin();

        auto from_advances = [&](bitboard &advance, int offset) {
            for (square index = 0; advance != 0 && index < 64;
                 ++index, advance >>= 1) {

                if (Utils::last_bit(advance)) {
                    move->piece_moved = Pieces::PAWNS;
                    move->from = index;

                    if (board.turn == Colors::WHITE) {
                        move->to = index + offset;
                    } else {
                        move->to = index - offset;
                    }

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

        auto from_captures = [&](bitboard &captures, int direction) {
            bitboard captures_by_piece[6];

            for (auto piece : Pieces::AllPieces) {
                captures_by_piece[piece] = captures & board.pieces[piece];
            }

            for (square index = 0; captures != 0 && index < 64; ++index) {
                if (Utils::last_bit(captures)) {
                    move->piece_moved = Pieces::PAWNS;
                    move->to = index;

                    if (board.turn == Colors::WHITE) {
                        move->from = index - 8 - direction;
                    } else {
                        move->from = index + 8 - direction;
                    }

                    for (auto piece : Pieces::AllPieces) {
                        if (Utils::last_bit(captures_by_piece[piece])) {
                            move->piece_captured = piece;
                            break;
                        }
                    }

                    if (move == moves.end() && captures != 1) {
                        throw std::out_of_range(
                            "Moves generated exceeded allocated space");
                    }
                    std::advance(move, 1);
                }

                captures >>= 1;
                for (auto piece : Pieces::AllPieces)
                    captures_by_piece[piece] >>= 1;
            }
        };

        from_captures(east_captures, -1);
        from_captures(west_captures, +1);

        return moves;
    }

    std::vector<Move> gen_rooks_moves(Game::Board board) {
        return Helpers::moves_from_generator<Magic::Rooks::get_avail_moves,
                                    Pieces::ROOKS>(board);
    }

    std::vector<Move> gen_bishops_moves(Game::Board board) {
        return Helpers::moves_from_generator<Magic::Bishops::get_avail_moves,
                                    Pieces::BISHOPS>(board);
    }

    std::vector<Move> gen_queens_moves(Game::Board board) {
        return Helpers::moves_from_generator<[](bitboard blockers, square index) {
            return Magic::Rooks::get_avail_moves(blockers, index) |
                   Magic::Bishops::get_avail_moves(blockers, index);
        },
                                    Pieces::QUEENS>(board);
    }

    std::vector<Move> gen_knights_moves(Game::Board board) {
        bitboard knights = board.knights & board.colors[board.turn];

        uint8_t knights_count = std::popcount(knights);

        uint16_t available_moves = 0;
        bitboard available_per_knight[knights_count];
        square knight_positions[knights_count];

        uint8_t current_knight = 0;

        for (square index = 0; knights != 0 && index < 64;
             ++index, knights >>= 1) {

            if (Utils::last_bit(knights)) {
                available_per_knight[current_knight] =
                    Knights::available_moves[index] & ~board.colors[board.turn];
                knight_positions[current_knight] = index;

                available_moves +=
                    std::popcount(available_per_knight[current_knight]);

                ++current_knight;
            }
        }

        std::vector<Move> moves(available_moves);

        if (moves.size() == 0) {
            return moves;
        }

        std::fill(moves.begin(), moves.end(),
                  Move{.piece_moved = Pieces::KNIGHTS});

        auto current_move = moves.begin();

        for (current_knight = 0; current_knight < knights_count;
             ++current_knight) {
            uint8_t size = std::popcount(available_per_knight[current_knight]);

            bitboard captures_by_type[7];
            captures_by_type[6] = 0;

            for (auto type : Pieces::AllPieces) {
                captures_by_type[type] =
                    available_per_knight[current_knight] & board.pieces[type];
                captures_by_type[6] |= captures_by_type[type];
            }

            Helpers::populate_from_bitboard(std::span(current_move, size),
                                   available_per_knight[current_knight],
                                   captures_by_type,
                                   knight_positions[current_knight]);

            std::advance(current_move, size);
        }

        return moves;
    }

} // namespace Game::Generators
