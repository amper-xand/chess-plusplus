#include "piecewise.hpp"

#include "../../../utils/utils.hpp"
#include <bit>

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

        bitboard e_attacks = Pawns::east_attacks(pawns, board.turn);
        bitboard w_attacks = Pawns::west_attacks(pawns, board.turn);

        bitboard east_captures = e_attacks & board.enemies();

        bitboard west_captures = w_attacks & board.enemies();

        bitboard e_enpassant = e_attacks & Utils::bit_at(board.enpassant_tail);
        bitboard w_enpassant = w_attacks & Utils::bit_at(board.enpassant_tail);

        auto total_available =
            std::popcount(single_advances) + std::popcount(double_advances) +
            std::popcount(east_captures) + std::popcount(west_captures) +
            std::popcount(e_enpassant) + std::popcount(w_enpassant);

        if (total_available == 0) {
            return generator;
        }

        auto moves_from_bitboard = [&](bitboard& bitboard,
                                       const auto& processor) {
            for (square index = 0; bitboard != 0 && index < 64;
                 ++index, bitboard >>= 1) {

                if (Utils::last_bit(bitboard)) {
                    Move& move = generator.next();

                    move.piece_moved = Pieces::PAWNS;

                    processor(move, index);
                }
            }
        };

        const auto advanced = [&](Move& move, square index, int offset) {
            move.from = index;
            move.to =
                (board.turn == Colors::WHITE) ? index + offset : index - offset;
        };

        const auto captured = [&](Move& move, square index, int direction) {
            move.to = index;
            move.from = (board.turn == Colors::WHITE) ? index - 8 - direction
                                                      : index + 8 - direction;
            move.piece_captured = board.piece_at(index);
        };

        // Process pawn advances
        moves_from_bitboard(single_advances, [&](Move& move, square index) {
            advanced(move, index, 8);
        });
        moves_from_bitboard(double_advances, [&](Move& move, square index) {
            move.enpassant_set = true;
            move.enpassant_capture = index;

            advanced(move, index, 16);
        });

        // Process pawn captures
        moves_from_bitboard(east_captures, [&](Move& move, square index) {
            captured(move, index, -1);
        });
        moves_from_bitboard(e_enpassant, [&](Move& move, square index) {
            move.enpassant_take = true;
            move.enpassant_capture = board.enpassant_capture;
            captured(move, index, -1);
        });

        moves_from_bitboard(west_captures, [&](Move& move, square index) {
            captured(move, index, 1);
        });
        moves_from_bitboard(w_enpassant, [&](Move& move, square index) {
            move.enpassant_take = true;
            move.enpassant_capture = board.enpassant_capture;
            captured(move, index, 1);
        });

        return generator;
    }

} // namespace Game::Generators::Pawns
