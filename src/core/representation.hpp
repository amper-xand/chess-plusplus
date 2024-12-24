#pragma once

#include <core/types.hpp>

#include <cstdint>
#include <string>

namespace core {

    struct Move {
        // clang-format off

        square from, to;

        Piece promotion = Piece::NONE;

        struct { Piece moved = Piece::NONE, captured = Piece::NONE; }
               piece;

        struct { bool set: 1 = false; bool take: 1 = false; square_t captured: 6 = 0; }
               enpassant;

        struct { bool take: 1 = false, reject: 1 = false; direction_t side: 1 = Direction::WEST; }
               castle;

        inline Move& same_as(Move other) {
            this->from = other.from;
            this->to = other.to;

            this->promotion = other.promotion;

            this->piece = other.piece;

            this->enpassant = other.enpassant;

            this->castle = other.castle;

            return *this;
        }
        // clang-format on
    };

    struct Board {
        // clang-format off

        static const uint8_t
            CAST_RIGHT = 0b100,
            CAST_WEST  = 0b010,
            CAST_EAST  = 0b001;

        union  { bitboard colors[2]; 
        struct { bitboard_t black, white; }; };

        union  { bitboard pieces[6];
        struct { bitboard_t pawns, knights, bishops, rooks, queens, kings; }; };

        struct { 
            uint8_t state = 0b111111;
                             
            inline void set(Color color, uint8_t state) 
                { this->state |= state << (3 & -color); }

            inline void off(Color color, uint8_t state) 
                { this->state &= ~(state << (3 & -color)); }

            inline uint8_t get(Color color) 
                { return (state >> (3 & -color)) & 0b111; }

            inline uint8_t flag(Direction dir) 
                { return dir.isWest() ? CAST_WEST : CAST_EAST; }

        } castling;

        struct { bool available = false; square_t capturable; square_t tail; }
               enpassant;

        Color turn;

        // clang-format on

        Board() : colors{bitboard(0)}, pieces{bitboard(0)} {}

        static Board parse_fen(std::string fen);

        void play(Move move);
        void unplay(Move move);
        void print();

        inline bitboard all() { return white | black; }

        inline bitboard allies() { return colors[turn]; }

        inline bitboard allied(Piece piece) {
            return pieces[piece] & colors[turn];
        }

        inline bitboard enemies() { return colors[!turn]; }

        inline bitboard enemy(Piece piece) {
            return pieces[piece] & colors[!turn];
        }

        inline bool is_occupied(square index) {
            return bitboard(white | black).is_set_at(index);
        }

        inline Color color_at(square index) {
            return Color::Both[bitboard(white).is_set_at(index)];
        }

        inline Piece piece_at(square index) {
            for (auto piece : Piece::All) {
                if (pieces[piece].is_set_at(index))
                    return piece;
            }

            return Piece::NONE;
        }

      private:
        void piece_move(Color color, Piece piece, bitboard from, bitboard to);
        void castle_update(Move move);
        void castle_take(Move move, bitboard king);
        void castle_undo(Move move);
        void enpassant_handle(Move move);
    };

} // namespace core
