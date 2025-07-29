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

void core::Board::play(const Move move) {
    assert(move.moved.isValid());

    // move the piece bit to its new position
    pieces[move.moved] ^= move.from.bb() | move.to.bb();
    colors[turn] ^= move.from.bb() | move.to.bb();

    if (!move.target.isNone()) {
        // remove the bit of the captured piece
        pieces[move.target] &= ~move.to.bb();
        colors[!turn] &= ~move.to.bb();
    }
}

void core::Board::unplay(const Move move) {
    assert(move.moved.isValid());

    // return the piece to its original position
    pieces[move.moved] ^= move.to.bb() | move.from.bb();
    colors[turn] ^= move.to.bb() | move.from.bb();

    if (!move.target.isNone()) {
        // return the captured piece
        pieces[move.target] |= move.to.bb();
        colors[!turn] |= move.to.bb();
    }
}
