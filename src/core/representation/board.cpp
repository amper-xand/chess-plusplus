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

        if (move.piece.captured != Pieces::NONE) {
            capture_piece(move.piece.captured, to);
        }

        handle_enpassant(move);

        // Switch turns
        turn = static_cast<Colors::Color>(!turn);
    }

    void Board::update_castling(Move move) {
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
    }

    void Board::take_castling(Move move, bitboard king) {
        // Move the rook
        bitboard rook =
            move.castle.west ? 0x0100000000000001 : 0x8000000000000080;

        rook.mask(allied(Pieces::ROOKS));

        bitboard rook_to = move.castle.west ? (king << 1) : (king >> 1);

        switch_bits(colors[turn], rook, rook_to);
        switch_bits(pieces[Pieces::ROOKS], rook, rook_to);
    }

    void Board::capture_piece(Pieces::Piece piece, bitboard captured) {
        colors[!turn] &= ~captured;
        colors[piece] &= ~captured;
    }

    void Board::handle_enpassant(Move move) {
        // Set en passant state
        enpassant.available = move.enpassant.set;

        if (move.enpassant.set) {
            enpassant.capturable = move.to;
            enpassant.tail =
                (turn == Colors::WHITE) ? move.to.down() : move.to.up();
        }

        if (move.enpassant.take) {
            capture_piece(Pieces::PAWNS, move.enpassant.captured);
        }
    }

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
                Colors::Color color = Colors::BothColors[std::isupper(c)];

                Pieces::Piece piece = Pieces::char_to_piece(c);

                board.colors[color] |= bitboard::bit_at(index);
                board.pieces[piece] |= bitboard::bit_at(index);
                index--;
            }
        }
        // clang-format on

        board.turn = Colors::WHITE;

        return board;
    }

    void Board::print() {
        std::cout << std::endl;

        // clang-format off
        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 7; file >= 0; --file) {

                square index = square::index_at(rank, file);

                auto square = is_occupied(index)
                  ? std::format(" {} ", Pieces::piece_to_char(piece_at(index), color_at(index)))
                  : " - ";

                std::cout << square;

                if (index % 8 == 0) std::cout << std::endl;
            }
        }
        // clang-format on

        std::cout << std::endl;
    }

} // namespace Game
