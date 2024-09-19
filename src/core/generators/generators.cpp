#include "generators.hpp"

#include "../../utils/utils.hpp"
#include "../game.hpp"
#include "helpers.hpp"
#include "magic/magic.hpp"

#include <bit>
#include <climits>
#include <cstdio>
#include <span>
#include <stdexcept>
#include <strings.h>
#include <vector>

using Game::Generators::MoveGenerator;

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

    bitboard get_available_moves(square index) {
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

    bitboard get_pinned_pieces(Board board) {
        bitboard king = board.allied(Pieces::KINGS);
        square position = std::countr_zero(king);

        bitboard diagonal_sliders =
            (board.bishops | board.queens) & board.enemies();

        bitboard straight_sliders =
            (board.bishops | board.queens) & board.enemies();

        bitboard pinned_pieces = 0;

        auto check_candidates = [&](bitboard candidates, auto slider_moves) {
            for (bitboard unchecked_bits = candidates; unchecked_bits != 0;
                 // Remove the candidate we just checked
                 unchecked_bits &= unchecked_bits - 1) {

                // Get the bits from the current bit (inclusive) to then end
                bitboard bits_previous = unchecked_bits ^ (unchecked_bits - 1);

                // Remove the bits before the current bit
                bitboard current_bit = bits_previous & unchecked_bits;

                // Make a hole by removing the candidates
                bitboard current_blockers = board.all_pieces() ^ current_bit;

                // If removing the candidate exposes the king to a slider
                auto is_pinned =
                    diagonal_sliders & slider_moves(board.allies(), position);

                // Then the piece is pinned
                if (is_pinned) {
                    pinned_pieces |= current_bit;
                }
            }
        };

        bitboard dia_candidates =
            board.allies() &
            Magic::Bishops::get_avail_moves(board.all_pieces(), position);

        bitboard str_candidates =
            board.allies() &
            Magic::Rooks::get_avail_moves(board.all_pieces(), position);

        check_candidates(dia_candidates, Magic::Bishops::get_avail_moves);
        check_candidates(str_candidates, Magic::Rooks::get_avail_moves);

        return pinned_pieces;
    }

    void initialize_table() {
        for (square index = 0; index < 64; ++index) {
            available_moves[index] = get_available_moves(index);
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

    std::vector<Move> generate_moves(Board board) {
        MoveGenerator generator(board);

        generator.pinned = Kings::get_pinned_pieces(board);

        gen_pawns_moves(generator);
        gen_bishops_moves(generator);
        gen_knights_moves(generator);
        gen_rooks_moves(generator);
        gen_queens_moves(generator);
        gen_king_moves(generator);

        return generator.get_generated();
    }

    MoveGenerator& gen_pawns_moves(MoveGenerator& generator) {
        Board& board = generator.board;

        bitboard blockers = board.all_pieces(),

                 pawns = board.allied(Pieces::PAWNS);

        // Remove pinned pawns
        pawns &= ~generator.pinned;

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

        auto total_available =
            std::popcount(single_advances) + std::popcount(double_advances) +
            std::popcount(east_captures) + std::popcount(west_captures);

        if (total_available == 0) {
            return generator;
        }

        Move& move = generator.next();

        auto moves_from_bitboard = [&](bitboard& bitboard,
                                       const auto& processor) {
            for (square index = 0; bitboard != 0 && index < 64;
                 ++index, bitboard >>= 1) {

                if (Utils::last_bit(bitboard)) {
                    move.piece_moved = Pieces::PAWNS;

                    processor(index);

                    move = generator.next();
                }
            }
        };

        const auto advanced = [&](square index, int offset) {
            move.from = index;
            move.to =
                (board.turn == Colors::WHITE) ? index + offset : index - offset;
        };

        const auto captured = [&](square index, int direction) {
            move.to = index;
            move.from = (board.turn == Colors::WHITE) ? index - 8 - direction
                                                      : index + 8 - direction;
            move.piece_captured = board.piece_at(index);
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

        return generator;
    }

    MoveGenerator& gen_rooks_moves(MoveGenerator& generator) {
        return Helpers::moves_from_generator<Magic::Rooks::get_avail_moves,
                                             Pieces::ROOKS>(generator);
    }

    MoveGenerator& gen_bishops_moves(MoveGenerator& generator) {
        return Helpers::moves_from_generator<Magic::Bishops::get_avail_moves,
                                             Pieces::BISHOPS>(generator);
    }

    MoveGenerator& gen_queens_moves(MoveGenerator& generator) {
        auto queen_generator = [](bitboard blockers, square index) {
            return Magic::Rooks::get_avail_moves(blockers, index) |
                   Magic::Bishops::get_avail_moves(blockers, index);
        };

        return Helpers::moves_from_generator<queen_generator, Pieces::QUEENS>(
            generator);
    }

    MoveGenerator& gen_knights_moves(MoveGenerator& generator) {
        auto knight_generator = [](bitboard blockers, square index) {
            return Knights::available_moves[index];
        };

        return Helpers::moves_from_generator<knight_generator, Pieces::KNIGHTS>(
            generator);
    }

    MoveGenerator& gen_king_moves(MoveGenerator& generator) {
        auto& board = generator.board;

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

        auto total_available = std::popcount(bbmoves);

        if (total_available == 0) {
            return generator;
        }

        std::span<Move> moves = generator.next_n(total_available);

        Helpers::populate_from_bitboard(moves, bbmoves, captures, board,
                                        king_position);

        return generator;
    }

} // namespace Game::Generators

namespace Game::Generators {
    MoveGenerator::MoveGenerator(Game::Board board) { this->board = board; }

    std::vector<Move> MoveGenerator::get_generated() {
        if (next_move == moves.begin())
            // No moves have been generated
            return std::vector<Move>(0);

        return std::vector(moves.begin(), next_move);
    }

    Move& MoveGenerator::next() {
        if (next_move > moves.end()) {
            throw std::out_of_range(
                "The generator execeeded allocated space for moves.");
        }

        Move& move = *next_move;
        std::advance(next_move, 1);

        return move;
    }

    std::span<Move> MoveGenerator::next_n(int n) {
        std::span<Move> span(next_move, n);

        if (span.end().base() > moves.end()) {
            throw std::out_of_range(
                "The generator tried to yield more moves than remaining.");
        }

        std::advance(next_move, n);

        return span;
    }

} // namespace Game::Generators
  //
