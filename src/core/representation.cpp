#include "representation.hpp"
#include "core/types.hpp"

#include <cstdint>
#include <iostream>
#include <strings.h>

namespace core {
    void Board::play(const Move& move) {
        auto from = move.from().bb();
        auto to = move.to().bb();

        if (!move.captured().isNone()) {
            // remove captured piece
            // due to ep there might not be a piece there
            pieces[move.captured()] &= ~to;
            colors[!turn] &= ~to;
        }

        // if the piece is moved before the capture
        // then it might be removed from the bitboard
        pieces[move.moved()] ^= from | to;
        colors[turn] ^= from | to;

        update_castling(move, from, to);
        update_enpassant(move, from, to);

        if (!move.promotion().isNone()) {
            // remove the promoted pawn
            pawns &= ~to;
            // add the promoted piece
            pieces[move.promotion()] |= to;
        }

        turn = !turn;
    }

    void Board::unplay(const Move& move) {
        // go back to the perspective of the player that made the move
        turn = !turn;

        auto from = move.from().bb();
        auto to = move.to().bb();

        // return the piece to its previous position
        // due to castling
        // the piece is not guaranteed to be in its target square
        pieces[move.moved()] = pieces[move.moved()].pop(to) | from;
        colors[turn] = colors[turn].pop(to) | from;

        // return the captured pieces
        if (move.captured() != Piece::NONE) {
            pieces[move.captured()] |= to;
            colors[!turn] |= to;
        }

        restore_castling(move, from, to);
        restore_enpassant(move, from, to);

        if (!move.promotion().isNone()) {
            pieces[move.promotion()] &= ~to;
        }
    }

    inline void Board::update_castling(const Move& move, //
                                       bitboard from, bitboard to) {
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

        // take castle
        if (move.cas_tag()) {

            // pieces look up tables

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

            // filter to the correct castling side and color
            bitboard side = bitboard::interval(from, to);

            bitboard rook_to = rook_castlings & side;
            bitboard king_to = king_castlings & side;

            // move the rook to its castle position
            colors[turn] ^= to | rook_to;
            rooks ^= to | rook_to;

            // correctly re-place the king
            kings ^= allied(Piece::KINGS) | king_to;
            colors[turn] ^= allied(Piece::KINGS) | king_to;
        }
    }

    inline void Board::update_enpassant(const Move& move, //
                                        bitboard from, bitboard to) {
        uint8_t ep_tag = move.ep_tag();

        // clear ep state
        enpassant.available = false;
        enpassant.pawn = 0;

        // set en passant
        if (ep_tag && move.captured().isNone()) {
            enpassant.available = true;
            enpassant.tail = turn ? move.to().down() : move.to().up();
            enpassant.pawn = move.to();
        }

        // take en passant
        if (ep_tag && move.captured().isPawn()) {
            auto capture = square(enpassant.pawn).bb();

            // remove the captured pawn
            colors[!turn] ^= capture;
            pawns ^= capture;
            enpassant.pawn = 0;
        }
    }

    inline void Board::restore_castling(const Move& move, //
                                        bitboard from, bitboard to) {
        // restore castling state
        castling.set_state(turn, move.castle());

        // undo castle
        if (move.cas_tag()) {
            // remove the king and place it again
            colors[turn] ^= allied(Piece::KINGS) | from;
            kings ^= allied(Piece::KINGS) | from;

            // take the rook on the castle side
            bitboard rook = rooks & bitboard::interval(from, to);

            // and move it to its origin
            colors[turn] ^= rook | to;
            rooks ^= rook | to;
        }
    }

    inline void Board::restore_enpassant(const Move& move, //
                                         bitboard from, bitboard to) {
        // get saved state
        uint8_t ep_tag = move.ep_tag();
        square ep = move.ep();

        // restore state
        enpassant.pawn = ep;
        // restore the tail of the enemy pawn
        enpassant.tail = turn ? ep.up() : ep.down();
        // reactivate if available
        enpassant.available = ep;

        // restore taken ep pawn
        if (ep_tag && move.captured().isPawn()) {
            auto capture = square(enpassant.pawn).bb();

            // place the captured pawn
            // and remove piece that was added when undoing the capture
            colors[!turn] ^= capture | to;
            pawns ^= capture | to;
        }
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
