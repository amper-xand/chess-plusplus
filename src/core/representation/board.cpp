#include "representation.hpp"

#include "../../utils/utils.hpp"
#include <format>
#include <iostream>
#include <strings.h>

namespace Game {
    bool Board::is_square_occupied(square index) {
        return Utils::is_set_at(index, white | black);
    }

    Colors::Color Board::color_at(square index) {
        return Colors::BothColors[Utils::is_set_at(index, white)];
    }

    Pieces::Piece Board::piece_at(square index) {
        for (auto piece : Pieces::AllPieces) {
            if (Utils::is_set_at(index, pieces[piece]))
                return piece;
        }

        return Pieces::NONE;
    }

    void Board::make_move(Move move) {
        // Change the position of the moved piece
        auto switch_bits = [](bitboard& target, bitboard from, bitboard to) {
            target &= ~from;
            target |= to;
        };

        auto from = Utils::bit_at(move.from), to = Utils::bit_at(move.to);

        switch_bits(colors[turn], from, to);
        switch_bits(pieces[move.piece.moved], from, to);

        // Handle captures
        auto capture_piece = [&](Pieces::Piece piece, bitboard captured) {
            colors[!turn] &= ~captured;
            colors[move.piece.captured] &= captured;
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
        turn = Colors::BothColors[!turn];
    }

    Board Board::parse_fen_string(std::string fen) {
        Board board;

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

                board.colors[color] |= Utils::bit_at(index);
                board.pieces[piece] |= Utils::bit_at(index);
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

                int index = Utils::index_at(rank, file);

                auto square =
                    is_square_occupied(index)
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
