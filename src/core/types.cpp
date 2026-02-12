#include <core/types.hpp>

#include <cassert>

/*
 * Applies a `Move` to the state of a `Board`.
 *
 * Does not perform any validation of the legality of the move.
 *
 * Returns the `Board::State` of the `Board` before applying the move.
 */
const core::Board::State core::Board::play(const Move move) {
    assert(move.moved.isValid());

    State& state = static_cast<State&>(*this);
    State prev = state;

    // move the piece bit to its new position
    colors[active_color] &= ~move.from.bb();
    colors[active_color] |= move.to.bb();
    pieces[move.moved] &= ~move.from.bb();
    pieces[move.moved] |= move.to.bb();

    // increase counter for 50 move rule
    ++state.halfmove_clock;

    // reset counter for 50 move rule
    if (!move.target.isNone() || move.moved.isPawn()) {
        state.halfmove_clock = 0;
    }

    // remove the bit of the captured piece
    if (!move.target.isNone()) {
        colors[!active_color] &= ~move.to.bb();
    }

    // clear the en en passant state
    state.en_passant_target_square = square::out_of_bounds;

    // capture en passant
    if (move.en_passant && !move.target.isNone()) {
        square capture = active_color ? move.to.down() : move.to.up();

        colors[!active_color] &= ~capture.bb();
    }

    // set en passant due to double advance
    if (move.en_passant && move.target.isNone()) {
        state.en_passant_target_square = active_color ? move.from.up() : move.from.down();
    }

    // promote pawn, switch bitboard
    if (!move.promotion.isNone()) {
        pieces[move.moved] ^= move.to.bb();
        pieces[move.promotion] |= move.to.bb();
    }

    // if castling move the rook to the correct position
    // update the castling state
    if (move.castle) {
        bitboard rook_movement;

        // get the bits needed to switch the rook
        rook_movement =
            active_color ? bitboard::masks::rank(0) : bitboard::masks::rank(7);

        if (move.from > move.to) {
            rook_movement &=
                bitboard::masks::file(0) | bitboard::masks::file(2);

            set_castling_right(false);
        } else {
            rook_movement &=
                bitboard::masks::file(7) | bitboard::masks::file(4);

            set_castling_left(false);
        }

        rooks ^= rook_movement;
        colors[active_color] ^= rook_movement;
    }

    // if the king is moved lose both castling sides
    if (move.moved.isKing()) {
        set_castling_left(false);
        set_castling_right(false);
    }

    // if a rook is moved in a corner,
    // then remove that castling right
    if (move.moved.isRook()) {
        if (active_color.isWhite() && move.from == square::at(0, 0)) {
            set_castling_right(false);
        }

        if (active_color.isWhite() && move.from == square::at(0, 7)) {
            set_castling_left(false);
        }

        if (active_color.isBlack() && move.from == square::at(7, 0)) {
            set_castling_right(false);
        }

        if (active_color.isBlack() && move.from == square::at(7, 7)) {
            set_castling_left(false);
        }
    }

    // clear stray bits left after captures
    bitboard all_pieces = all();

    for (bitboard& bb : pieces) {
        bb &= all_pieces;
    }

    active_color = !active_color;

    return prev;
}

/*
 * Reverses the effects of a `Move`.
 *
 * Takes the last `Move` applied and the previous `Board::State`.
 *
 * Does not perform any validation of the `Move` or `Board::State`.
 */
void core::Board::unplay(const Move move, const State prev) {
    assert(move.moved.isValid());

    // return the board to its previous state
    static_cast<State&>(*this) = prev;

    // move the piece to its original position
    // if there is a promotion this adds an extra pawn *
    colors[active_color] &= ~move.to.bb();
    colors[active_color] |= move.from.bb();
    pieces[move.moved] &= ~move.to.bb();
    pieces[move.moved] |= move.from.bb();

    // if a piece was captured then add it back
    if (!move.target.isNone() && !move.en_passant) {
        pieces[move.target] |= move.to.bb();
        colors[!active_color] |= move.to.bb();
    }

    // if a pawn was captured via en passant
    // then put it back
    if (!move.target.isNone() && move.en_passant) {
        square pawn = active_color ? move.to.down() : move.to.up();
        pieces[move.target] |= pawn.bb();
        colors[!active_color] |= pawn.bb();
    }

    // if there was a promotion
    // delete the promoted piece
    if (!move.promotion.isNone()) {
        pieces[move.promotion] ^= move.to.bb();
    }

    // if there was a castle then
    // move the rook back to its original position
    if (move.castle) {
        bitboard rook_movement;

        rook_movement =
            active_color ? bitboard::masks::rank(0) : bitboard::masks::rank(7);

        if (move.from > move.to) {
            rook_movement &=
                bitboard::masks::file(0) | bitboard::masks::file(2);
        } else {
            rook_movement &=
                bitboard::masks::file(7) | bitboard::masks::file(4);
        }

        rooks ^= rook_movement;
        colors[active_color] ^= rook_movement;
    }
}
