#include "representation.hpp"

#include <format>
#include <iostream>
#include <strings.h>

namespace Game {
    inline void switch_bits(bitboard& target, bitboard from, bitboard to) {
        target = target.pop(from).join(to);
    };

    void Board::play(Move move) {
        auto from = bitboard::bit_at(move.from);
        auto to = bitboard::bit_at(move.to);

        switch_bits(colors[turn], from, to);
        switch_bits(pieces[move.piece.moved], from, to);

        // Handle castling
        update_castling(move);

        if (move.castle.take) {
            take_castling(move, from);
        }

        if (!move.piece.captured.isNone()) {
            capture_piece(move.piece.captured, to);
        }

        handle_enpassant(move);

        turn = !turn;
    }

    void Board::update_castling(Move move) {
        if (move.piece.moved.isKing()) {
            if (turn.isWhite()) {
                castling.white = false;
            } else {
                castling.black = false;
            }
        }

        if (move.piece.moved.isRook()) {
            if (move.from == 0) {
                (turn ? castling.white_east : castling.black_east) = false;
            } else if (move.from == 7) {
                (turn ? castling.white_west : castling.black_west) = false;
            }
        }
    }

    void Board::take_castling(Move move, bitboard king) {
        // Move the rook
        bitboard rook =
            move.castle.west ? 0x0100000000000001 : 0x8000000000000080;

        rook = rook.mask(allied(Piece::ROOKS));

        bitboard rook_to = move.castle.west ? (king << 1) : (king >> 1);

        switch_bits(colors[turn], rook, rook_to);
        switch_bits(pieces[Piece::ROOKS], rook, rook_to);
    }

    void Board::capture_piece(Piece piece, bitboard captured) {
        colors[!turn] &= ~captured;
        colors[piece] &= ~captured;
    }

    void Board::handle_enpassant(Move move) {
        // Set en passant state
        enpassant.available = move.enpassant.set;

        if (move.enpassant.set) {
            enpassant.capturable = move.to;
            enpassant.tail = turn.isWhite() ? move.to.down() : move.to.up();
        }

        if (move.enpassant.take) {
            capture_piece(Piece::PAWNS, move.enpassant.captured);
        }
    }

    bool Board::is_occupied(square index) {
        return static_cast<bitboard>(white | black).is_set_at(index);
    }

    Color Board::color_at(square index) {
        return Color::BothColors[static_cast<bitboard>(white).is_set_at(index)];
    }

    Piece Board::piece_at(square index) {
        for (auto piece : Piece::AllPieces) {
            if (pieces[piece].is_set_at(index))
                return piece;
        }

        return Piece::NONE;
    }

    void Board::promote(Piece promotion, bitboard to) {
        pieces[promotion] |= to;
        pawns &= ~to;
    }

    void Board::unplay(Move move) {
        turn = !turn;

        auto from = bitboard::bit_at(move.from);
        auto to = bitboard::bit_at(move.to);

        // return the piece to its previous position
        switch_bits(colors[turn], to, from);
        switch_bits(pieces[move.piece.moved], to, from);

        if (move.piece.captured != Piece::NONE) {
            pieces[move.piece.captured] |= to;
            colors[!turn] |= to;
        }

        // check if move rejected ep
        enpassant.available = !move.enpassant.take && move.enpassant.captured;

        if (move.castle.take) {
            uncastle(move);
        }

        if (move.castle.reject) {
            turn ? castling.white : castling.black = true;
        }

        if (!move.promotion.isNone()) {
            pieces[move.promotion] &= ~to;
        }
    }

    void Board::uncastle(Move move) {
        if (turn) {
            castling.white = true;
            (move.castle.west ? castling.white_west : castling.white_east) =
                true;
        } else {
            castling.black = true;
            (move.castle.west ? castling.black_west : castling.black_east) =
                true;
        }
    }

    Board Board::from_fen(std::string fen) {
        Board board;

        // TODO: Parse other aspects about board state

        square index = 63;

        // clang-format off
        for (char c : fen) {
            if (index > 64) { break; }

            if (std::isdigit(c)) { index -= c - '0'; // Skip empty squares.
            } else if (c == '/') { continue; // Move to the next rank.
                                             
            } else {
                // Fill piece.
                Color color = Color::BothColors[std::isupper(c)];

                Piece piece = Piece::char_to_piece(c);

                board.colors[color] |= bitboard::bit_at(index);
                board.pieces[piece] |= bitboard::bit_at(index);
                index--;
            }
        }
        // clang-format on

        board.turn = Color::WHITE;

        return board;
    }

    void Board::print() {
        std::cout << std::endl;

        // clang-format off
        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 7; file >= 0; --file) {

                square index = square::index_at(rank, file);

                auto square = is_occupied(index)
                  ? std::format(" {} ", Piece::piece_to_char(piece_at(index), color_at(index)))
                  : " - ";

                std::cout << square;

                if (index % 8 == 0) std::cout << std::endl;
            }
        }
        // clang-format on

        std::cout << std::endl;
    }

} // namespace Game
