#pragma once

#include "types.hpp"
#include "enums.hpp"

#include <string>

namespace Game {
    struct Move {
        // clang-format off

        square from, to;

        struct { Pieces::Piece moved, captured = Pieces::NONE; } piece;

        struct { bool set : 1 = false; bool take: 1 = false; square captured: 6; } enpassant;

        // clang-format on
    };
} // namespace Game

namespace Game {
    struct Board {
        // clang-format off

        Colors::Color turn;

        union { bitboard colors[2]; struct { bitboard black, white; }; };

        union { bitboard pieces[6]; struct { bitboard pawns, knights, bishops, rooks, queens, kings; }; };

        struct { bool available : 1 = false; square capturable : 6; square tail : 6; }
               enpassant;
        // clang-format on

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
