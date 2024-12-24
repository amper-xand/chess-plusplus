#include "representation.hpp"

#include <iostream>
#include <strings.h>

namespace core {

    void Board::play(Move move) {
        auto from = move.from.bb();
        auto to = move.to.bb();

        piece_move(turn, move.piece.moved, from, to);

        castle_update(move);

        if (move.castle.take) {
            castle_take(move, from);
        }

        if (!move.piece.captured.isNone()) {
            colors[!turn] &= ~to;
            colors[move.piece.captured] &= ~to;
        }

        enpassant_handle(move);

        if (!move.promotion.isNone()) {
            pieces[move.promotion] |= to;
            pawns &= ~to;
        }

        turn = !turn;
    }

    void Board::unplay(Move move) {
        turn = !turn;

        auto from = move.from.bb();
        auto to = move.to.bb();

        // return the piece to its previous position
        piece_move(turn, move.piece.moved, to, from);

        if (move.piece.captured != Piece::NONE) {
            pieces[move.piece.captured] |= to;
            colors[turn] |= to;
        }

        // check if move rejected ep
        enpassant.available = !move.enpassant.take && move.enpassant.captured;

        if (move.castle.take) {
            castle_undo(move);
        }

        if (move.castle.reject) {
            castling.set(turn, CAST_RIGHT);
        }

        if (!move.promotion.isNone()) {
            pieces[move.promotion] &= ~to;
        }
    }

    void Board::castle_update(Move move) {
        if (move.piece.moved.isKing()) {
            castling.off(turn, Board::CAST_RIGHT);
        }

        if (move.piece.moved.isRook()) {
            castling.off(turn, castling.flag(move.from.column() == 7));
        }
    }

    void Board::castle_take(Move move, bitboard king) {
        // Move the rook
        bitboard rook =
            move.castle.side ? 0x0100000000000001 : 0x8000000000000080;

        rook = rook.mask(allied(Piece::ROOKS));

        bitboard rook_to = move.castle.side ? (king << 1) : (king >> 1);

        piece_move(turn, Piece::ROOKS, rook, rook_to);
    }

    void Board::castle_undo(Move move) {
        castling.set(turn, CAST_RIGHT | castling.flag(move.castle.side));
    }

    void Board::enpassant_handle(Move move) {
        // Set en passant state
        enpassant.available = move.enpassant.set;

        if (move.enpassant.set) {
            enpassant.capturable = move.to;
            enpassant.tail = turn.isWhite() ? move.to.down() : move.to.up();
        }

        if (move.enpassant.take) {
            auto capture = square(move.enpassant.captured).bb();

            colors[!turn] &= ~capture;
            colors[move.piece.captured] &= ~capture;
        }
    }

    inline void Board::piece_move(Color color, Piece piece, bitboard from,
                                  bitboard to) {
        pieces[piece] = pieces[piece].pop(from).join(to);
        colors[color] = colors[color].pop(from).join(to);
    }

    Board Board::parse_fen(std::string fen) {
        Board board;

        square index = 63;

        for (char c : fen) {
            if (index > 64)
                break;

            if (c == '/')
                continue; // Move to the next rank.

            if (std::isdigit(c)) {
                index -= c - '0'; // Skip empty squares.
                continue;
            }

            Color color = Color::Both[std::isupper(c)];

            Piece piece = Piece::char_to_piece(c);

            board.colors[color] |= bitboard::bit_at(index);
            board.pieces[piece] |= bitboard::bit_at(index);
            index--;
        }

        board.turn = Color::WHITE;

        // TODO: Parse other aspects about board state

        return board;
    }

    void Board::print() {
        for (int rank = 7; rank >= 0; --rank) {
            std::cout << std::endl;
            for (int file = 7; file >= 0; --file) {

                square index = square::index_at(rank, file);

                auto square =
                    Piece::piece_to_char(piece_at(index), color_at(index));

                std::cout << ' ' << square << ' ';
            }
            std::cout << std::endl;
        }
    }

    Piece Piece::char_to_piece(char c) {
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

        return PAWNS;
        // clang-format on
    }

    char Piece::piece_to_char(Piece piece, Color color) {
        // clang-format off
        switch (piece) {

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

} // namespace core
