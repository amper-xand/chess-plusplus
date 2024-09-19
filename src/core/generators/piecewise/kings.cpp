#include "piecewise.hpp"

#include "../../../utils/utils.hpp"
#include "../helpers.hpp"
#include "../magic/magic.hpp"

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

} // namespace Game::Generators::Kings
