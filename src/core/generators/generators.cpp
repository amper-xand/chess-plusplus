#include "generators.hpp"

#include "../../utils/utils.hpp"
#include "../game.hpp"
#include "helpers.hpp"
#include "magic.hpp"

#include <bit>
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

namespace Game::Generators::Kings {
    bitboard available_moves[64];

    bitboard gen_king_moves(square index) {
        bitboard moves = 0, position = Utils::bit_at(index);

        square col = Utils::column(index);

        moves |= position << 8;
        moves |= position >> 8;

        if (col >= 1) {
            moves |= moves >> 1;
            moves |= position >> 1;
        }

        if (col <= 6) {
            moves |= moves << 1;
            moves |= position << 1;
        }

        return moves;
    }

    void initialize_table() {
        for (square index = 0; index < 64; ++index) {
            available_moves[index] = gen_king_moves(index);
        }
    }
} // namespace Game::Generators::Kings

namespace Game::Generators::Pawns {
    // clang-format off
    bitboard get_advances(bitboard pawns, bitboard blockers, bool color) {

        if (color == Colors::WHITE) pawns <<= 8;
        else                        pawns >>= 8;

        return pawns & ~blockers;
    }

    bitboard get_advances(bitboard pawns, bitboard blockers, Colors::Color color) {
        return get_advances(pawns, blockers, (bool) color);
    }

    bitboard east_attacks(bitboard pawns, bool color) {
        // Filter pawns that will wrap around the board
        bitboard attacks = pawns & ~0x0101010101010101;

        if (color == Colors::WHITE) {
            attacks <<= 7; // up + right
        } else {
            attacks >>= 9; // down + right
        }

        return attacks;
    }

    bitboard east_attacks(bitboard pawns, Colors::Color color) {
        return east_attacks(pawns, (bool) color);
    }

    bitboard west_attacks(bitboard pawns, bool color) {
        // Filter pawns that will wrap around the board
        bitboard attacks = pawns & ~0x8080808080808080;

        if (color == Colors::WHITE) {
            attacks <<= 9; // up + left
        } else {
            attacks >>= 7; // down + left
        }

        return attacks;
    }

    bitboard west_attacks(bitboard pawns, Colors::Color color) {
        return west_attacks(pawns, (bool) color);
    }
    // clang-format on
} // namespace Game::Generators::Pawns

namespace Game::Generators {
    void initialize_tables() {
        Knights::initialize_table();
        Kings::initialize_table();
        Magic::initialize_magic_tables();
    }

    std::vector<Move> gen_pawn_moves(Board board) {
        bitboard blockers = board.all_pieces(),

                 pawns = board.allied(Pieces::PAWNS);

        bitboard single_advances =
            Pawns::get_advances(pawns, blockers, board.turn);

        bitboard double_advances =
            single_advances &
            // If a pawn advanced to the 3rd or 6th rank, advance it again
            (board.turn == Colors::WHITE ? 0x0000000000FF0000
                                         : 0x0000FF0000000000);

        double_advances =
            Pawns::get_advances(double_advances, blockers, board.turn);

        // return pawns to their original positions
        if (board.turn == Colors::WHITE) {
            single_advances >>= 8;
            double_advances >>= 16;
        } else {
            single_advances <<= 8;
            double_advances <<= 16;
        }

        bitboard east_captures =
            Pawns::east_attacks(pawns, board.turn) & board.enemies();

        bitboard west_captures =
            Pawns::west_attacks(pawns, board.turn) & board.enemies();

        std::vector<Move> moves(
            std::popcount(single_advances) + std::popcount(double_advances) +
            std::popcount(east_captures) + std::popcount(west_captures));

        if (moves.size() == 0) {
            return moves;
        }

        auto move = moves.begin();

        auto moves_from_bitboard = [&](bitboard &bitboard,
                                       const auto &processor) {
            for (square index = 0; bitboard != 0 && index < 64;
                 ++index, bitboard >>= 1) {

                if (Utils::last_bit(bitboard)) {
                    processor(index);

                    move->piece_moved = Pieces::PAWNS;

                    if (move == moves.end() && bitboard != 1) {
                        throw std::out_of_range(
                            "Moves generated exceeded allocated space");
                    }

                    std::advance(move, 1);
                }
            }
        };

        const auto advanced = [&](square index, int offset) {
            move->from = index;
            move->to =
                (board.turn == Colors::WHITE) ? index + offset : index - offset;
        };

        const auto captured = [&](square index, int direction) {
            move->to = index;
            move->from = (board.turn == Colors::WHITE) ? index - 8 - direction
                                                       : index + 8 - direction;
            move->piece_captured = board.piece_at(index);
        };

        // Process pawn advances without direction parameter
        moves_from_bitboard(single_advances,
                            [&](square index) { advanced(index, 8); });
        moves_from_bitboard(double_advances,
                            [&](square index) { advanced(index, 16); });

        // Process pawn captures with direction parameter
        moves_from_bitboard(east_captures,
                            [&](square index) { captured(index, -1); });
        moves_from_bitboard(west_captures,
                            [&](square index) { captured(index, 1); });

        return moves;
    }

    std::vector<Move> gen_rooks_moves(Board board) {
        return Helpers::moves_from_generator<Magic::Rooks::get_avail_moves,
                                             Pieces::ROOKS>(board);
    }

    std::vector<Move> gen_bishops_moves(Board board) {
        return Helpers::moves_from_generator<Magic::Bishops::get_avail_moves,
                                             Pieces::BISHOPS>(board);
    }

    std::vector<Move> gen_queens_moves(Board board) {
        auto queen_generator = [](bitboard blockers, square index) {
            return Magic::Rooks::get_avail_moves(blockers, index) |
                   Magic::Bishops::get_avail_moves(blockers, index);
        };

        return Helpers::moves_from_generator<queen_generator, Pieces::QUEENS>(
            board);
    }

    std::vector<Move> gen_knights_moves(Board board) {
        auto knight_generator = [](bitboard blockers, square index) {
            return Knights::available_moves[index];
        };

        return Helpers::moves_from_generator<knight_generator, Pieces::KNIGHTS>(
            board);
    }

    std::vector<Move> gen_king_moves(Board board) {
        square king_position = std::countr_zero(board.allied(Pieces::KINGS));

        bitboard diagonal_sliders =
                     (board.bishops | board.queens) & board.enemies(),

                 straight_sliders =
                     (board.rooks | board.queens) & board.enemies(),

                 knights = board.enemy(Pieces::KNIGHTS);

        bitboard attacked_squares = 0, blockers = board.all_pieces();

        for (square index = 0; index < 64; ++index) {

            if (Utils::last_bit(diagonal_sliders)) {
                attacked_squares |=
                    Magic::Bishops::get_avail_moves(blockers, index);
            }

            if (Utils::last_bit(straight_sliders)) {
                attacked_squares |=
                    Magic::Rooks::get_avail_moves(blockers, index);
            }

            if (Utils::last_bit(straight_sliders)) {
                attacked_squares |= Knights::available_moves[index];
            }

            diagonal_sliders >>= 1;
            straight_sliders >>= 1;
            knights >>= 1;
        }

        attacked_squares |=
            Pawns::west_attacks(board.enemy(Pieces::PAWNS), !board.turn);

        attacked_squares |=
            Pawns::west_attacks(board.enemy(Pieces::PAWNS), !board.turn);

        // Add the other kings's attacks
        attacked_squares |= Kings::available_moves[std::countr_zero(
            board.enemy(Pieces::KINGS))];

        bitboard bbmoves = Kings::available_moves[king_position];

        // Remove attacked and blocked squares
        bbmoves &= ~attacked_squares;
        bbmoves &= ~board.allies();

        bitboard captures = (board.enemies() & ~attacked_squares) & bbmoves;

        std::vector<Move> moves(std::popcount(bbmoves));

        if (moves.size() == 0) {
            return moves;
        }

        Helpers::populate_from_bitboard(std::span(moves.begin(), moves.size()),
                                        bbmoves, captures, board,
                                        king_position);

        return moves;
    }

} // namespace Game::Generators
