#include <core/types.hpp>

#include <cassert>
#include <print>

core::Piece core::Piece::from_character_repr(char c) {
    // clang-format off
        switch (c) {
        case 'p': case 'P':
            return PAWNS;

        case 'n': case 'N':
            return KNIGHTS;

        case 'b': case 'B':
            return BISHOPS;

        case 'r': case 'R':
            return ROOKS;

        case 'q': case 'Q':
            return QUEENS;

        case 'k': case 'K':
            return KINGS;
        }

        return NONE;
    // clang-format on
}

char core::Piece::to_character_repr(Color color) {
    // clang-format off
    switch (this->value_) {
        case PAWNS   : return color ? 'P' : 'p';
        case KNIGHTS : return color ? 'N' : 'n';
        case BISHOPS : return color ? 'B' : 'b';
        case ROOKS   : return color ? 'R' : 'r';
        case QUEENS  : return color ? 'Q' : 'q';
        case KINGS   : return color ? 'K' : 'k';

        case NONE: return '-';
    }

    return 'X';
    // clang-format on
}

// FIX ME: rows are being parsed in reverse
core::Board core::Board::parse_fen_repr(std::string fen) {
    Board board;

    square index = 63;

    for (char c : fen) {
        if (index > 64) break;

        if (c == '/') continue;  // Move to the next rank.

        if (std::isdigit(c)) {
            index -= c - '0';  // Skip empty squares.
            continue;
        }

        // !! to ensure value is 0 or 1
        Color color = !!std::isupper(c);

        Piece piece = Piece::from_character_repr(c);

        assert(piece != Piece::NONE);

        board.colors[color] |= bitboard::masks::at(index);
        board.pieces[piece] |= bitboard::masks::at(index);
        index--;
    }

    board.turn = Color::WHITE;

    // TODO: Parse other aspects about board state

    return board;
}

void core::Board::print() {
    for (int rank = 7; rank >= 0; --rank) {
        std::print("\n");
        for (int file = 7; file >= 0; --file) {
            square index = square::at(rank, file);

            auto piece_char = piece(index).to_character_repr(color(index));

            std::print(" {} ", piece_char);
        }
    }
    std::print("\n");
}

/*
 * Applies a `Move` to the state of a `Board`.
 *
 * Does not perform any validation of the legality of the move.
 *
 * Returns the `Board::State` of the `Board` before applying the move.
 */
const core::Board::State core::Board::play(const Move move) {
    assert(move.moved.isValid());

    State prev = state;

    // move the piece bit to its new position
    colors[turn] &= ~move.from.bb();
    colors[turn] |= move.to.bb();
    pieces[move.moved] &= ~move.from.bb();
    pieces[move.moved] |= move.to.bb();

    // increase counter for 50 move rule
    ++state.fifty_move_rule_counter;

    // reset counter for 50 move rule
    if (!move.target.isNone() || move.moved.isPawn()) {
        state.fifty_move_rule_counter = 0;
    }

    // remove the bit of the captured piece
    if (!move.target.isNone()) {
        colors[!turn] &= ~move.to.bb();
    }

    // clear the en en passant state
    state.en_passant = 65;

    // capture en passant
    if (move.en_passant && !move.target.isNone()) {
        square capture = turn ? move.to.down() : move.to.up();

        colors[!turn] &= ~capture.bb();
    }

    // set en passant due to double advance
    if (move.en_passant && move.target.isNone()) {
        state.en_passant = turn ? move.from.up() : move.from.down();
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
            turn ? bitboard::masks::rank(0) : bitboard::masks::rank(7);

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
        colors[turn] ^= rook_movement;
    }

    // if the king is moved lose both castling sides
    if (move.moved.isKing()) {
        set_castling_left(false);
        set_castling_right(false);
    }

    // if a rook is moved in a corner,
    // then remove that castling right
    if (move.moved.isRook()) {
        if (turn.isWhite() && move.from == square::at(0, 0)) {
            set_castling_right(false);
        }

        if (turn.isWhite() && move.from == square::at(0, 7)) {
            set_castling_left(false);
        }

        if (turn.isBlack() && move.from == square::at(7, 0)) {
            set_castling_right(false);
        }

        if (turn.isBlack() && move.from == square::at(7, 7)) {
            set_castling_left(false);
        }
    }

    // clear stray bits left after captures
    bitboard all_pieces = all();

    for (bitboard& bb : pieces) {
        bb &= all_pieces;
    }

    turn = !turn;

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
    state = prev;
    turn = !turn;

    // move the piece to its original position
    // if there is a promotion this adds an extra pawn *
    colors[turn] &= ~move.to.bb();
    colors[turn] |= move.from.bb();
    pieces[move.moved] &= ~move.to.bb();
    pieces[move.moved] |= move.from.bb();

    // if a piece was captured then add it back
    if (!move.target.isNone() && !move.en_passant) {
        pieces[move.target] |= move.to.bb();
        colors[!turn] |= move.to.bb();
    }

    // if a pawn was captured via en passant
    // then put it back
    if (!move.target.isNone() && move.en_passant) {
        square pawn = turn ? move.to.down() : move.to.up();
        pieces[move.target] |= pawn.bb();
        colors[!turn] |= pawn.bb();
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
            turn ? bitboard::masks::rank(0) : bitboard::masks::rank(7);

        if (move.from > move.to) {
            rook_movement &=
                bitboard::masks::file(0) | bitboard::masks::file(2);
        } else {
            rook_movement &=
                bitboard::masks::file(7) | bitboard::masks::file(4);
        }

        rooks ^= rook_movement;
        colors[turn] ^= rook_movement;
    }
}
