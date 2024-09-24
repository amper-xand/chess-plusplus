#include "piecewise.hpp"

#include "../../../utils/utils.hpp"
#include <bit>
#include <cstdio>

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
        // TODO: Implement promotions
        Board& board = generator.board;

        bitboard blockers = board.all_pieces(),

                 pawns = board.allied(Pieces::PAWNS);

        // Remove pinned pawns
        pawns &= ~generator.pinned;

        struct {
            bitboard singles, doubles;
        } advances;

        advances.singles = Pawns::get_advances(pawns, blockers, board.turn);

        advances.doubles =
            advances.singles &
            // If a pawn advanced to the 3rd or 6th rank, advance it again
            (board.turn == Colors::WHITE ? 0x0000000000FF0000
                                         : 0x0000FF0000000000);

        advances.doubles =
            Pawns::get_advances(advances.doubles, blockers, board.turn);

        // return pawns to their original positions
        if (board.turn == Colors::WHITE) {
            advances.singles >>= 8;
            advances.doubles >>= 16;
        } else {
            advances.singles <<= 8;
            advances.doubles <<= 16;
        }

        struct {
            bitboard east = 0, west = 0;
        } attacks, captures, enpassant;

        attacks.east = Pawns::east_attacks(pawns, board.turn);
        attacks.west = Pawns::west_attacks(pawns, board.turn);

        captures.east = attacks.east & board.enemies();
        captures.west = attacks.west & board.enemies();

        if (board.enpassant.available && generator.enpassant.pinned) {
            enpassant.east = attacks.east & Utils::bit_at(board.enpassant.tail);
            enpassant.west = attacks.west & Utils::bit_at(board.enpassant.tail);
        }

        auto total_available =
            std::popcount(advances.singles) + std::popcount(advances.doubles) +
            std::popcount(captures.east) + std::popcount(captures.west) +
            std::popcount(enpassant.east) + std::popcount(enpassant.west);

        if (total_available == 0) {
            return generator;
        }

        auto moves_from_bitboard = [&](bitboard& bitboard,
                                       const auto& processor) {
            for (square index = 0; bitboard != 0 && index < 64;
                 ++index, bitboard >>= 1) {

                if (Utils::last_bit(bitboard)) {
                    Move& move = generator.next();

                    move.piece.moved = Pieces::PAWNS;

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
            move.piece.captured = board.piece_at(index);
        };

        // Process pawn advances
        moves_from_bitboard(advances.singles, [&](Move& move, square index) {
            advanced(move, index, 8);
        });
        moves_from_bitboard(advances.doubles, [&](Move& move, square index) {
            move.enpassant.set = true;

            advanced(move, index, 16);
        });

        // Process pawn captures
        moves_from_bitboard(captures.east, [&](Move& move, square index) {
            captured(move, index, -1);
        });
        moves_from_bitboard(enpassant.east, [&](Move& move, square index) {
            move.enpassant.take = true;
            move.enpassant.captured = board.enpassant.capturable;
            captured(move, index, -1);
        });

        moves_from_bitboard(captures.west, [&](Move& move, square index) {
            captured(move, index, 1);
        });
        moves_from_bitboard(enpassant.west, [&](Move& move, square index) {
            move.enpassant.take = true;
            move.enpassant.captured = board.enpassant.capturable;
            captured(move, index, 1);
        });

        return generator;
    }

} // namespace Game::Generators::Pawns
