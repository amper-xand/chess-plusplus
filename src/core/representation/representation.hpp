#pragma once

#include "enums.hpp"
#include "types.hpp"

#include <string>

namespace Game {
    struct Move {
        // clang-format off

        square from, to;

        Pieces::Piece promotion = Pieces::NONE;

        struct { Pieces::Piece moved, captured = Pieces::NONE; }
               piece;

        struct { bool set: 1 = false; bool take: 1 = false; square_t captured: 6 = 0; }
               enpassant;

        struct { bool take: 1 = false, west: 1 = true;}
               castle;

        inline Move& copy(Move other) {
            this->from = other.from;
            this->to = other.to;

            this->piece = other.piece;

            this->promotion = other.promotion;

            this->enpassant = other.enpassant;

            this->castle = other.castle;

            return *this;
        }
        // clang-format on
    };
} // namespace Game

namespace Game {
    struct Board {
        // clang-format off

        Colors::Color turn;

        struct { bool white_west: 1 = true, white_east: 1 = true,
                      black_west: 1 = true, black_east: 1 = true; }
               castling;

        struct { bool available : 1 = false; square_t capturable : 6; square_t tail : 6; }
               enpassant;

        union { bitboard colors[2]; 
               struct { bitboard_t black, white; }; };

        union { bitboard pieces[6];
               struct { bitboard_t pawns, knights, bishops, rooks, queens, kings; }; };

        // clang-format on

        Board() : colors{bitboard(0)}, pieces{bitboard(0)} {}

        static Board from_fen(std::string fen);

        // Squares state

        bool is_occupied(square index);
        Colors::Color color_at(square index);
        Pieces::Piece piece_at(square index);

        // Get bitboards

        inline bitboard all() { return white | black; }

        inline bitboard allies() { return colors[turn]; }
        inline bitboard allied(Pieces::Piece piece) {
            return pieces[piece] & colors[turn];
        }

        inline bitboard enemies() { return colors[!turn]; }
        inline bitboard enemy(Pieces::Piece piece) {
            return pieces[piece] & colors[!turn];
        }

        // State

        inline bool west_castle() const {
            return turn ? castling.white_west : castling.black_west;
        }

        inline bool east_castle() const {
            return turn ? castling.white_east : castling.black_east;
        }

        void play(Move move);
        void update_castling(Move move);
        void take_castling(Move move, bitboard king);
        void capture_piece(Pieces::Piece piece, bitboard captured);
        void handle_enpassant(Move move);
        void promote(Pieces::Piece promotion, bitboard to);

        void unplay(Move move);
        void uncastle(Move move);

        void print();
    };

} // namespace Game
