#include "piecewise.hpp"

#include "../magic/magic.hpp"

namespace core::generators::kings {
    bitboard available_moves[64];

    bitboard get_available_moves(square index) {
        bitboard moves = 0, position = bitboard::bitpos(index);

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

    void initialize_table() {
        for (square index = 0; index < 64; ++index) {
            available_moves[index] = get_available_moves(index);
        }
    }

    template <piece_t piece>
    bitboard pin_xray(const Board& board, bitboard direction) {
        bitboard king = board.allied(Piece::KINGS);

        bitboard pinnable =
            board.allies() | board.pawns; // pin enemy pawns for ep
        bitboard blockers = board.enemies();

        bitboard sliders = board.enemies()
                               .mask(board.pieces[piece] | board.queens)
                               .mask(direction);

        // Make a ray a zero it out if there are no sliders

        bitboard before = sliders.mask(king - 1).MSB();
        before = bitboard::interval(king, before) & -before;

        bitboard after = sliders.pop(king - 1).LSB();
        after = bitboard::interval(king, after) * (after != 0);

        // block and check if the rays are pinning a single piece
        bitboard pinned;

        pinned = pinnable.mask(before);
        before *=
            (blockers.mask(before) == 0) && ((pinned & (pinned - 1)) == 0);

        pinned = pinnable.mask(after);
        after *= (blockers.mask(after) == 0) && ((pinned & (pinned - 1)) == 0);

        return ((before | after) ^ king) & direction;
    }

    bitboard pin_rook_xrays(const Board& board) {
        bitboard king = board.allied(Piece::KINGS);
        square position = king.rzeros();

        return pin_xray<Piece::ROOKS>(board, bitboard::masks::horizontal
                                                 << position.row_start()) |
               pin_xray<Piece::ROOKS>(board, bitboard::masks::vertical
                                                 << position.column());
    }

    bitboard pin_bishop_xrays(const Board& board) {
        bitboard king = board.allied(Piece::KINGS);
        square position = king.rzeros();

        return pin_xray<Piece::BISHOPS>(
                   board, bitboard::masks::get_diagonal_at(position)) |
               pin_xray<Piece::BISHOPS>(
                   board, bitboard::masks::get_rev_diagonal_at(position));
    }

    bool is_enpassant_pinned(MoveGenerator& generator) {
        if (!generator.board.enpassant.available)
            return false;

        return generator.pins.absolute.bit(generator.board.enpassant.pawn);
    }

    bitboard get_attacked_squares(const Board& board) {
        bitboard diagonal_sliders =
                     board.enemies().mask(board.bishops | board.queens),

                 straight_sliders =
                     board.enemies().mask(board.rooks | board.queens),

                 knights = board.enemy(Piece::KNIGHTS);

        bitboard attacked_squares = 0, blockers = board.all();

        for (square index = 0; index < 64; ++index) {

            if (diagonal_sliders.bit0()) {
                attacked_squares |=
                    magic::bishops::get_avail_moves(blockers, index);
            }

            if (straight_sliders.bit0()) {
                attacked_squares |=
                    magic::rooks::get_avail_moves(blockers, index);
            }

            if (straight_sliders.bit0()) {
                attacked_squares |= knights::available_moves[index];
            }

            diagonal_sliders >>= 1;
            straight_sliders >>= 1;
            knights >>= 1;
        }

        attacked_squares |=
            pawns::west_attacks(board.enemy(Piece::PAWNS), !board.turn);

        attacked_squares |=
            pawns::west_attacks(board.enemy(Piece::PAWNS), !board.turn);

        // Add the other kings's attacks
        attacked_squares |=
            kings::available_moves[board.enemy(Piece::KINGS).rzeros()];

        return attacked_squares;
    }

    void handle_castling(MoveGenerator& generator, square king_position,
                         bitboard attacked_squares) {

        const Board& board = generator.board;

        auto castling = board.castling.get_state(board.turn);

        if (!(castling & Board::CAST_RIGHT))
            return;

        if (castling & Board::CAST_WEST) {
            bitboard cast_squares = 0b11111000;

            if (board.turn.isBlack()) {
                cast_squares <<= square::at(7, 0);
            }

            if (cast_squares ==
                // Check that the castling path is not obstructed
                cast_squares.mask(!(board.all() | attacked_squares))) {

                Move& move = generator.next();
                move.moved(Piece::KINGS);
                move.cas_tag(true);

                move.from(king_position);
                move.to(king_position.left(2));
            }
        }

        if (castling & Board::CAST_EAST) {
            bitboard cast_squares = 0b00001111;

            if (board.turn.isBlack()) {
                cast_squares <<= square::at(7, 0);
            }

            if (cast_squares ==
                // Check that the castling path is not obstructed
                cast_squares.mask(!(board.all() | attacked_squares))) {

                Move& move = generator.next();
                move.moved(Piece::KINGS);
                move.cas_tag(true);

                move.from(king_position);
                move.to(king_position.right(2));
            }
        }
    }

    template <bool generate_castle>
    void gen_king_moves(MoveGenerator& generator) {
        auto& board = generator.board;

        square king_position = board.allied(Piece::KINGS).rzeros();

        bitboard attacked_squares = get_attacked_squares(board);

        if constexpr (generate_castle)
            handle_castling(generator, king_position, attacked_squares);

        // Generate the rest of the moves
        bitboard moves = kings::available_moves[king_position];

        // Remove attacked and blocked squares
        moves &= ~attacked_squares;
        moves &= ~board.allies();

        bitboard captures = board.enemies().mask(moves);

        auto castle = board.castling.get_state(board.turn);

        for (square index = 0; moves != 0;
             moves <<= 1, captures <<= 1, ++index) {

            if (moves.bit0()) {
                auto& move = generator.next();

                move.moved(Piece::KINGS);

                move.from(king_position);
                move.to(index);

                if (captures.bit0())
                    move.captured(board.piece(index));
            }
        }
    }

    template void gen_king_moves<true>(MoveGenerator& generator);
    template void gen_king_moves<false>(MoveGenerator& generator);

    bitboard checking_pieces(const Board& board) {
        bitboard king = board.allied(Piece::KINGS);

        square king_pos = king.rzeros();

        bitboard checks;

        checks |= board.enemy(Piece::KNIGHTS)
                      .mask(knights::available_moves[king_pos]);

        checks |= magic::rooks::get_avail_moves(board.all(), king_pos)
                      .mask(board.rooks | board.queens)
                      .mask(board.enemies());

        checks |= magic::bishops::get_avail_moves(board.all(), king_pos)
                      .mask(board.bishops | board.queens)
                      .mask(board.enemies());

        bitboard pawns_attackers = // west attackers
            king.pop(0x8080808080808080) << 1;

        pawns_attackers |= // east attackers
            king.pop(0x0101010101010101) >> 1;

        pawns_attackers = pawns_attackers.forward(board.turn, 8);

        checks |= pawns_attackers.mask(board.enemy(Piece::PAWNS));

        return checks;
    }

} // namespace core::generators::kings
