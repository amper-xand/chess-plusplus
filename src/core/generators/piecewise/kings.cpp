#include "piecewise.hpp"

#include "../helpers.hpp"
#include "../magic/magic.hpp"
#include <algorithm>
#include <tuple>

namespace Game::Generators::Kings {
    bitboard available_moves[64];

    bitboard get_available_moves(square index) {
        bitboard moves = 0, position = bitboard::bit_at(index);

        square col = index.column();

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

    std::tuple<bitboard, bitboard, bitboard> get_pinned_pieces(Board board) {
        bitboard king = board.allied(Pieces::KINGS);
        square position = king.rzeros();

        bitboard diagonal_sliders =
            board.enemies().mask(board.bishops | board.queens);

        // Remove sliders that already have line of sight to the king
        diagonal_sliders &=
            ~Magic::Bishops::get_avail_moves(board.all(), position);

        bitboard straight_sliders =
            board.enemies().mask(board.rooks | board.queens);

        // Remove sliders that already have line of sight to the king
        straight_sliders &=
            ~Magic::Rooks::get_avail_moves(board.all(), position);

        bitboard pinned_pieces = 0, pinners = 0;

        auto check_candidates = [&](bitboard candidates, bitboard sliders,
                                    const auto& slider_moves) {
            for (bitboard unchecked_bits = candidates; unchecked_bits != 0;
                 // Remove the candidate we just checked
                 unchecked_bits &= unchecked_bits - 1) {

                // Get the bits from the current bit (inclusive) to then end
                bitboard bits_previous = unchecked_bits ^ (unchecked_bits - 1);

                // Remove the bits before the current bit
                bitboard current_bit = bits_previous.mask(unchecked_bits);

                // Make a hole by removing the candidate
                bitboard current_blockers = board.all() ^ current_bit;

                // If removing the candidate exposes the king to a slider
                auto is_pinned =
                    sliders.mask(slider_moves(current_blockers, position));

                // Then the piece is pinned
                if (is_pinned != 0) {
                    pinners |= is_pinned; // Store pinner
                    pinned_pieces |= current_bit;
                }
            }
        };

        // Determine which are the pinned pieces

        bitboard dia_candidates =
            board.allies() &
            Magic::Bishops::get_avail_moves(board.all(), position);

        bitboard str_candidates =
            board.allies() &
            Magic::Rooks::get_avail_moves(board.all(), position);

        check_candidates(dia_candidates, diagonal_sliders,
                         Magic::Bishops::get_avail_moves);
        check_candidates(str_candidates, straight_sliders,
                         Magic::Rooks::get_avail_moves);

        // Determine relative pins

        bitboard partial_pins = 0;

        partial_pins = pinned_pieces
                           .mask(dia_candidates)
                           // Get diagonaly pinned pieces that can capture back
                           .mask(board.bishops | board.queens);

        partial_pins |= pinned_pieces
                            .mask(str_candidates)
                            // Get pinned in cross pieces that can capture back
                            .mask(board.rooks | board.queens);

        partial_pins |= pinned_pieces
                            .mask(dia_candidates)
                            // Get diagonaly pinned pawns
                            .mask(board.pawns);

        bitboard absolute_pins = pinned_pieces.mask(~partial_pins);

        return {absolute_pins, partial_pins, pinners};
    }

    bool is_enpassant_pinned(Board board) {
        if (!board.enpassant.available)
            return false;

        bitboard king = board.allied(Pieces::KINGS);
        square position = king.rzeros();

        bitboard diagonal_sliders =
            board.enemies().mask(board.bishops | board.queens);

        // Remove sliders that already have line of sight to the king
        diagonal_sliders &=
            ~Magic::Bishops::get_avail_moves(board.all(), position);

        bitboard straight_sliders =
            board.enemies().mask(board.rooks | board.queens);

        // Remove sliders that already have line of sight to the king
        straight_sliders &=
            ~Magic::Rooks::get_avail_moves(board.all(), position);

        // Get blockers without the enpassant pawn
        bitboard blockers =
            board.all() ^ bitboard::bit_at(board.enpassant.capturable);

        // Check for discovered attacks
        bitboard is_pinned;

        is_pinned = diagonal_sliders.mask(
            Magic::Bishops::get_avail_moves(blockers, position));

        is_pinned |= straight_sliders.mask(
            Magic::Rooks::get_avail_moves(blockers, position));

        return is_pinned != 0;
    }

    void initialize_table() {
        for (square index = 0; index < 64; ++index) {
            available_moves[index] = get_available_moves(index);
        }
    }

    bitboard get_attacked_squares(Board& board) {
        bitboard diagonal_sliders =
                     board.enemies().mask(board.bishops | board.queens),

                 straight_sliders =
                     board.enemies().mask(board.rooks | board.queens),

                 knights = board.enemy(Pieces::KNIGHTS);

        bitboard attacked_squares = 0, blockers = board.all();

        for (square index = 0; index < 64; ++index) {

            if (diagonal_sliders.last_bit()) {
                attacked_squares |=
                    Magic::Bishops::get_avail_moves(blockers, index);
            }

            if (straight_sliders.last_bit()) {
                attacked_squares |=
                    Magic::Rooks::get_avail_moves(blockers, index);
            }

            if (straight_sliders.last_bit()) {
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
        attacked_squares |=
            Kings::available_moves[board.enemy(Pieces::KINGS).rzeros()];

        return attacked_squares;
    }

    MoveGenerator& gen_king_moves(MoveGenerator& generator, bool gencastle) {
        auto& board = generator.board;

        square king_position = board.allied(Pieces::KINGS).rzeros();

        bitboard attacked_squares = get_attacked_squares(board);

        // Generate castling
        if (gencastle && board.can_castle(true)) {
            bitboard cast_squares = 0b11111000;

            if (!board.turn) {
                cast_squares <<= square::index_at(7, 0);
            }

            if (cast_squares ==
                // Check that the castling path is not obstructed
                cast_squares.mask(!(board.all() | attacked_squares))) {

                Move& move = generator.next();
                move.piece.moved = Pieces::KINGS;
                move.castle = {.take = true, .west = false};

                move.from = king_position;
                move.to = king_position.left(2);
            }
        }

        if (gencastle && board.can_castle(false)) {
            bitboard cast_squares = 0b00001111;

            if (!board.turn) {
                cast_squares <<= square::index_at(7, 0);
            }

            if (cast_squares ==
                // Check that the castling path is not obstructed
                cast_squares.mask(!(board.all() | attacked_squares))) {

                Move& move = generator.next();
                move.piece.moved = Pieces::KINGS;
                move.castle = {.take = true, .west = true};

                move.from = king_position;
                move.to = king_position.right(2);
            }
        }
        // Generate the rest of the moves
        bitboard king_moves = Kings::available_moves[king_position];

        // Remove attacked and blocked squares
        king_moves &= ~attacked_squares;
        king_moves &= ~board.allies();

        bitboard captures =
            board.enemies().mask(~attacked_squares).mask(king_moves);

        auto total_available = king_moves.popcount();

        if (total_available == 0) {
            return generator;
        }

        std::span<Move> moves = generator.next_n(total_available);

        std::fill(moves.begin(), moves.end(),
                  Move{.piece{.moved = Pieces::KINGS}});

        Helpers::populate_from_bitboard(moves, king_moves, captures, board,
                                        king_position);

        return generator;
    }

    bitboard checking_pieces(Board& board) {

        square king = board.allied(Pieces::KINGS).rzeros();

        bitboard checks;

        checks |=
            board.enemy(Pieces::KNIGHTS).mask(Knights::available_moves[king]);

        checks |= Magic::Rooks::get_avail_moves(board.all(), king)
                      .mask(board.rooks | board.queens);

        checks |= Magic::Bishops::get_avail_moves(board.all(), king)
                      .mask(board.bishops | board.queens);

        bitboard pawns_attackers = // west attackers
            board.allied(Pieces::KINGS).pop(0x8080808080808080) << 1;

        pawns_attackers |= // east attackers
            board.allied(Pieces::KINGS).pop(0x0101010101010101) >> 1;

        if (board.turn) {
            pawns_attackers <<= 8;
        } else {
            pawns_attackers >>= 8;
        }

        checks |= pawns_attackers;

        return checks;
    }

} // namespace Game::Generators::Kings
