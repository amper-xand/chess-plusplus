#pragma once

#include "enums.hpp"
#include "types.hpp"

#include <string>

namespace Game {
    struct Move {
        // clang-format off

        square from, to;

        struct { Pieces::Piece moved, captured = Pieces::NONE; } piece;

        Pieces::Piece promotion = Pieces::NONE;

        struct { bool set : 1 = false; bool take: 1 = false; square_t captured: 6; } enpassant;

        inline Move& copy(Move other) {
            this->from = other.from;
            this->to = other.to;

            this->piece = other.piece;

            this->promotion = other.promotion;

            this->enpassant = other.enpassant;

            return *this;
        }
        // clang-format on
    };
} // namespace Game

namespace Game {
    struct Board {
        // clang-format off

        Colors::Color turn;

        union { bitboard colors[2]; struct { bitboard_t black, white; }; };

        union { bitboard pieces[6]; struct { bitboard_t pawns, knights, bishops, rooks, queens, kings; }; };

        struct { bool available : 1 = false; square_t capturable : 6; square_t tail : 6; }
               enpassant;
        // clang-format on

        Board() : colors{bitboard(0)}, pieces{bitboard(0)} {}

        static Board parse_fen_string(std::string fen);

        void print();

        bool is_square_occupied(square index);

        Colors::Color color_at(square index);

        Pieces::Piece piece_at(square index);

        inline bitboard all_pieces() { return white | black; }

        inline bitboard allies() { return colors[turn]; }

        inline bitboard allied(Pieces::Piece piece) {
            return pieces[piece] & colors[turn];
        }

        inline bitboard enemies() { return colors[!turn]; }

        inline bitboard enemy(Pieces::Piece piece) {
            return pieces[piece] & colors[!turn];
        }

        void make_move(Move move);
    };

} // namespace Game
