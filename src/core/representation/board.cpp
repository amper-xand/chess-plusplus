#include "representation.hpp"

#include <format>
#include <iostream>
#include <strings.h>

namespace Game {
    bool Board::is_occupied(square index) {
        return static_cast<bitboard>(white | black).is_set_at(index);
    }

    Colors::Color Board::color_at(square index) {
        return Colors::BothColors[static_cast<bitboard>(white).is_set_at(
            index)];
    }

    Pieces::Piece Board::piece_at(square index) {
        for (auto piece : Pieces::AllPieces) {
            if (pieces[piece].is_set_at(index))
                return piece;
        }

        return Pieces::NONE;
    }

    void Board::play(Move move) {
        // Change the position of the moved piece
        auto switch_bits = [](bitboard& target, bitboard from, bitboard to) {
            target &= ~from;
            target |= to;
        };

        auto from = bitboard::bit_at(move.from), to = bitboard::bit_at(move.to);

        switch_bits(colors[turn], from, to);
        switch_bits(pieces[move.piece.moved], from, to);

        // Handle castling
        if (move.piece.moved == Pieces::KINGS) {
            if (turn == Colors::WHITE) {
                castling.white_east = false;
                castling.white_west = false;
            } else {
                castling.black_east = false;
                castling.black_west = false;
            }
        }

        if (move.piece.moved == Pieces::ROOKS) {
            if (move.from == 0) {
                (turn ? castling.white_east : castling.black_east) = false;
            } else if (move.from == 7) {
                (turn ? castling.white_west : castling.black_west) = false;
            }
        }

        if (move.castle.take) {
            // Move the rook
            bitboard rook =
                move.castle.west ? 0x0100000000000001 : 0x8000000000000080;

            rook.mask(allied(Pieces::ROOKS));

            bitboard rook_to = move.castle.west ? (from << 1) : (from >> 1);

            switch_bits(colors[turn], rook, rook_to);
            switch_bits(pieces[Pieces::ROOKS], rook, rook_to);
        }

        // Handle captures
        auto capture_piece = [&](Pieces::Piece piece, bitboard captured) {
            colors[!turn] &= ~captured;
            colors[move.piece.captured] &= ~captured;
        };

        if (move.piece.captured != Pieces::NONE) {
            capture_piece(move.piece.captured, to);
        }

        // Set en passant state
        enpassant.available = move.enpassant.set;

        if (move.enpassant.set) {
            enpassant.capturable = move.to;
            enpassant.tail =
                (turn == Colors::WHITE) ? move.to - 8 : move.to + 8;
        }

        if (move.enpassant.take) {
            capture_piece(Pieces::PAWNS, move.enpassant.captured);
        }

        // Switch turns
        turn = static_cast<Colors::Color>(!turn);
    }

    Board Board::from_fen(std::string fen) {
        Board board;

        // TODO: Parse other aspects about board state

        square index = 63;

        for (char c : fen) {
            if (index > 64) {
                break;
            }

            if (std::isdigit(c)) {
                // Skip empty squares.
                index -= c - '0';
            } else if (c == '/') {
                // Move to the next rank.
                continue;
            } else {
                // Fill piece.
                Colors::Color color = Colors::BothColors[std::isupper(c)];

                Pieces::Piece piece = Pieces::char_to_piece(c);

                board.colors[color] |= bitboard::bit_at(index);
                board.pieces[piece] |= bitboard::bit_at(index);
                index--;
            }
        }

        board.turn = Colors::WHITE;

        return board;
    }

    void Board::print() {
        std::cout << std::endl;

        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 7; file >= 0; --file) {

                square index = square::index_at(rank, file);

                auto square =
                    is_occupied(index)
                        ? std::format(" {} ",
                                      Pieces::piece_to_char(piece_at(index),
                                                            color_at(index)))
                        : " - ";

                std::cout << square;

                if (index % 8 == 0)
                    std::cout << std::endl;
            }
        }

        std::cout << std::endl;
    }

} // namespace Game
