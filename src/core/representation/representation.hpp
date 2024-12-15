#pragma once

#include "types.hpp"

#include <cstdint>
#include <string>

namespace Game {
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
        Color turn;

        // clang-format off
        static const uint8_t
            CAST_RIGHT = 0b100,
            CAST_WEST  = 0b010,
            CAST_EAST  = 0b001;

        struct { 
            uint8_t state = 0b111111;
                             
            inline void set(Color color, uint8_t state) { this->state |= state << (3 & -color); }
            inline void off(Color color, uint8_t state) { this->state &= ~(state << (3 & -color)); }
            inline uint8_t get(Color color) { return (state >> (3 & -color)) & 0b111; }

            inline uint8_t flag(Direction dir) { return dir.isWest() ? CAST_WEST : CAST_EAST; }

        } castling;

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
        inline bitboard all() { return white | black; }

        inline bitboard allies() { return colors[turn]; }

        inline bitboard allied(Piece piece) {
            return pieces[piece] & colors[turn];
        }

        inline bitboard enemies() { return colors[!turn]; }

        inline bitboard enemy(Piece piece) {
            return pieces[piece] & colors[!turn];
        }

        void play(Move move);

        void unplay(Move move);

        void print();

      private:
        inline void piece_move(Color color, Piece piece, bitboard from,
                               bitboard to) {
            pieces[piece] = pieces[piece].pop(from).join(to);
            colors[color] = colors[color].pop(from).join(to);
        }

        inline void piece_capture(Piece piece, bitboard captured) {
        }

        inline void piece_promote(Piece promotion, bitboard to) {
            pieces[promotion] |= to;
            pawns &= ~to;
        }

        void castle_update(Move move);
        void castle_take(Move move, bitboard king);
        void castle_undo(Move move);

        void enpassant_handle(Move move);
    };

} // namespace Game
