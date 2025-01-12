#include "representation.hpp"
#include "core/types.hpp"

#include <cstdint>
#include <iostream>
#include <strings.h>

namespace core {
    void Board::play(Move move) {
        auto from = move.from().bb();
        auto to = move.to().bb();

        piece_move(turn, move.moved(), from, to);

        // handle castling

        // remove castling right
        if (move.moved().isKing()) {
            castling.remove_right(turn, CAST_RIGHT);
        }

        // remove castling right
        if (move.moved().isRook()) {
            if (move.from().column() == 0) {
                castling.remove_right(turn, CAST_EAST);
            } else {
                castling.remove_right(turn, CAST_WEST);
            }
        }

        if (move.cas_tag()) {
            constexpr bitboard king_castlings =
                // king side castle
                square::index_at(0, 1).bb() | square::index_at(7, 1).bb() |
                // queen side castle
                square::index_at(0, 5).bb() | square::index_at(7, 5).bb();

            constexpr bitboard rook_castlings =
                // king side castle
                square::index_at(0, 2).bb() | square::index_at(7, 2).bb() |
                // queen side castle
                square::index_at(0, 4).bb() | square::index_at(7, 4).bb();

            bitboard side = bitboard::interval(from, to);

            bitboard rook_to = rook_castlings & side;
            bitboard king_to = king_castlings & side;

            piece_move(turn, Piece::ROOKS, to, rook_to);
            // move the king again to put it in place
            piece_move(turn, Piece::KINGS, to, king_to);
        }

        // handle captures
        if (!move.captured().isNone()) {
            colors[!turn] &= ~to;
            colors[move.captured()] &= ~to;
        }

        // handle en passant
        uint8_t ep_tag = move.ep_tag();
        enpassant.available = false;

        // set en passant
        if (ep_tag && move.captured().isNone()) {
            enpassant.available = true;
            enpassant.tail = turn ? move.to().down() : move.to().up();
            enpassant.pawn = move.to();
        }

        // take en passant
        if (ep_tag && move.captured().isPawn()) {
            auto capture = square(enpassant.pawn).bb();

            colors[!turn] &= ~capture;
            pawns &= ~capture;

            enpassant.pawn = 0;
        }

        // handle promotions
        if (!move.promotion().isNone()) {
            pieces[move.promotion()] |= to;
            pawns &= ~to;
        }

        turn = !turn;
    }

    void Board::unplay(Move move) {
        turn = !turn;

        auto from = move.from().bb();
        auto to = move.to().bb();

        // return the piece to its previous position
        piece_move(turn, move.moved(), to, from);

        // return the captured pieces
        if (move.captured() != Piece::NONE) {
            pieces[move.captured()] |= to;
            colors[!turn] |= to;
        }

        // restore castling state
        castling.set_state(turn, move.castle());

        // undo castle
        if (move.cas_tag()) {
            // return king to its place
            auto king = allied(Piece::KINGS) | from;

            colors[turn] ^= king;
            kings ^= king;

            // return the rook to its place
            auto rook = rooks & bitboard::interval(from, to);
            rook |= to;

            colors[turn] ^= rook;
            rooks ^= rook;
        }

        // get saved state
        uint8_t ep_tag = move.ep_tag();
        square ep = move.ep();

        // restore state
        enpassant.pawn = ep;
        enpassant.tail = turn ? ep.up() : ep.down();
        enpassant.available = ep;

        // restore taken en passant pawn
        if (ep_tag && move.captured().isPawn()) {
            auto capture = square(enpassant.pawn).bb();

            colors[!turn] |= capture;
            pawns |= capture;
        }

        if (!move.promotion().isNone()) {
            pieces[move.promotion()] &= ~to;
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

            Piece piece = Piece::from_char(c);

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

                auto square = Piece::to_char(piece_at(index), color_at(index));

                std::cout << ' ' << square << ' ';
            }
        }
        std::cout << std::endl;
    }

    Piece Piece::from_char(char c) {
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

    char Piece::to_char(Piece piece, Color color) {
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
