#include "generators.hpp"

#include "../../utils/utils.hpp"
#include "../game.hpp"
#include "helpers.hpp"
#include "magic.hpp"

#include <bit>
#include <cstdio>
#include <iterator>
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
        Knights::initialize_table();
        Magic::initialize_magic_tables();
    }

    std::vector<Move> gen_pawn_moves(Game::Board board) {
        bitboard blockers = board.all_pieces(),

                 allied_pawns = board.pawns & board.colors[board.turn],

                 single_advances = allied_pawns;

        if (board.turn == Colors::WHITE) {
            single_advances <<= 8;
        } else {
            single_advances >>= 8;
        }

        // Remove blocked pawns
        single_advances &= ~blockers;

        // If a pawn advanced to the 3rd or 6th rank, advance it again
        bitboard double_advances =
            single_advances &
            (board.turn == Colors::WHITE ? 0xFF0000
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
            single_advances >>= 8;
            double_advances >>= 16;
        } else {
            single_advances <<= 8;
            double_advances <<= 16;
        }

        // Filter pawns that will wrap around the board
        bitboard east_captures = allied_pawns & ~0x0101010101010101;

        if (board.turn == Colors::WHITE) {
            east_captures <<= 7; // up + right
        } else {
            east_captures >>= 9; // down + right
        }

        // Get pawns that are over an enemy piece
        east_captures &= board.colors[!board.turn];

        // Filter pawns that will wrap around the board
        bitboard west_captures = allied_pawns & ~0x8080808080808080;

        if (board.turn == Colors::WHITE) {
            west_captures <<= 9; // up + left
        } else {
            west_captures >>= 7; // down + left
        }

        // Get pawns that are over an enemy piece
        west_captures &= board.colors[!board.turn];

        std::vector<Move> moves(
            std::popcount(single_advances) + std::popcount(double_advances) +
            std::popcount(east_captures) + std::popcount(west_captures));

        if (moves.size() == 0) {
            return moves;
        }

        auto move = moves.begin();

        auto get_moves_from_advances = [&](bitboard &advance, int offset) {
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

                    // If it is not the last bit
                    // but it is end of the list throw an error
                    if (move == moves.end() && advance != 1) {
                        throw std::out_of_range(
                            "Moves generated exceeded allocated space");
                    }

                    std::advance(move, 1);
                }
            }
        };

        get_moves_from_advances(single_advances, 8);
        get_moves_from_advances(double_advances, 16);

        auto get_moves_from_captures = [&](bitboard &captures, int direction) {
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

                    // If it is not the last bit
                    // but it is end of the list throw an error
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

        get_moves_from_captures(east_captures, -1);
        get_moves_from_captures(west_captures, +1);

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
        auto queen_generator = [](bitboard blockers, square index) {
            return Magic::Rooks::get_avail_moves(blockers, index) |
                   Magic::Bishops::get_avail_moves(blockers, index);
        };

        return Helpers::moves_from_generator<queen_generator, Pieces::QUEENS>(
            board);
    }

    std::vector<Move> gen_knights_moves(Game::Board board) {
        auto knight_generator = [](bitboard blockers, square index) {
            return Knights::available_moves[index];
        };

        return Helpers::moves_from_generator<knight_generator, Pieces::KNIGHTS>(
            board);
    }

} // namespace Game::Generators
