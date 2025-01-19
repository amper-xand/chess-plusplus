#pragma once

#include <core/types.hpp>

#include <cstdint>
#include <string>

namespace core {

    struct Move : LiteralWrapper<uint32_t> {
        // clang-format off

        // from      6 bits offset 0
        // to        6 bits offset 6
        // moved     3 bits offset 12
        // captured  3 bits offset 15
        // promotion 3 bits offset 18
        // ep        6 bits offset 21  // save ep state
        // ep_tag    6 bits offset 22  // ep set or take flag
        // castle    3 bits offset 28  // save castle state
        // cas_tag   1 bits offset 31  // castle_take tag

        #define getter(type, name, size, offset) \
        inline type name() const {               \
            return get<type, size, offset>();    \
        }                                        \

        #define setter(name, size, offset) \
        inline void name(auto value) {     \
            set<size, offset>(value);      \
        }                                  \

        #define member(type, name, size, offset) \
        getter(type, name, size, offset)         \
        setter(name, size, offset)               \

        /*---------------------------*/
        member(square, from     , 6,  0)
        member(square, to       , 6,  6)
        member(Piece , moved    , 3, 12)
        member(Piece , captured , 3, 15)
        member(Piece , promotion, 3, 18)
        member(square, ep       , 6, 21)
        member(uint8_t,ep_tag   , 1, 27)
        member(uint8_t,castle   , 3, 28)
        member(uint8_t,cas_tag  , 1, 31)
        /*---------------------------*/

        #define mask(value, size, offset) ( \
        (((1u << size) - 1) << offset) &    \
        (value << offset)                 ) \

        static constexpr uint32_t default_mv = //
        mask(Piece::NONE, 3, 12) | mask(Piece::NONE, 3, 15) | mask(Piece::NONE, 3, 18);

        #undef mask

        #undef member
        #undef setter
        #undef getter

        constexpr Move(uint32_t value = default_mv )
           : LiteralWrapper<uint32_t>(value) {}

        inline Move& like(Move& other) {
            this->value_ = other.value_;
            return *this;
        }

      private:
        template <typename Type, uint8_t size, uint8_t offset>
        inline Type get() const {
            constexpr int mask = (1u << size) - 1;
            return (value_ >> offset) & mask;
        }

        template <uint8_t size, uint8_t offset>
        inline void set(uint32_t value) {
            constexpr int mask = ((1u << size) - 1) << offset;

            value <<= offset;

            value_ &= ~mask;
            value_ |= value & mask;
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
            // redo this
            uint8_t state = 0b111111;
                             
            inline void set_state(Color color, uint8_t state) 
                { this->state |= state << (3 & -color); }

            inline void remove_right(Color color, uint8_t state) 
                { this->state &= ~(state << (3 & -color)); }

            inline uint8_t get_state(Color color) 
                { return (state >> (3 & -color)) & 0b111; }

        } castling;

        struct { bool available = false; square_t pawn = 0; square_t tail; }
               enpassant;

        Color turn;

        // clang-format on

        Board() : colors{bitboard(0)}, pieces{bitboard(0)} {}

        static Board parse_fen(std::string fen);

        bool operator==(Board other);

        void play(const Move& move);
        void unplay(const Move& move);

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
        void update_castling(const Move& move, bitboard from, bitboard to);
        void restore_castling(const Move& move, bitboard from, bitboard to);

        void update_enpassant(const Move& move, bitboard from, bitboard to);
        void restore_enpassant(const Move& move, bitboard from, bitboard to);
    };

} // namespace core
