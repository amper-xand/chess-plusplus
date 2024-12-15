#pragma once

#include <bit>
#include <cstdint>
#include <functional>
#include <iostream>

namespace Game {

    typedef uint64_t bitboard_t;
    typedef uint8_t square_t;
    typedef uint8_t color_t;
    typedef uint8_t piece_t;
    typedef uint8_t direction_t;

    template <typename type> class LiteralWrapper {
      protected:
        type value_ = 0;

      public:
        constexpr LiteralWrapper(type value) : value_(value) {}

        // Overload operators to forward to the wrapped type
        constexpr operator type &() { return value_; }
        constexpr operator const type &() const { return value_; }

        // Forward other operations to the underlying type as needed
        constexpr type *operator->() { return &value_; }
        constexpr const type *operator->() const { return &value_; }
    };

    template <typename type> struct OperatorWrapper {
        inline constexpr type operator+(const auto& other) {
            return *this + other;
        }

        inline constexpr type operator-(const auto& other) {
            return *this + other;
        }

        inline constexpr type operator*(const auto& other) {
            return *this + other;
        }

        inline constexpr type operator/(const auto& other) {
            return *this + other;
        }

        inline constexpr type operator&(const auto& other) {
            return *this + other;
        }

        inline constexpr type operator|(const auto& other) {
            return *this + other;
        }

        inline constexpr type operator^(const auto& other) {
            return *this + other;
        }
    };

    struct square : public LiteralWrapper<square_t>,
                    public OperatorWrapper<square> {
      public:
        constexpr square(square_t value = 0)
            : LiteralWrapper<square_t>(value) {}

        inline constexpr uint8_t column() { return value_ % 8; }

        inline constexpr uint8_t row() { return value_ / 8; }

        inline constexpr square right(uint8_t v = 1) { return value_ - v; }
        inline constexpr square left(uint8_t v = 1) { return value_ + v; }
        inline constexpr square up(uint8_t v = 1) { return value_ + 8 * v; }
        inline constexpr square down(uint8_t v = 1) { return value_ - 8 * v; }

        inline constexpr bitboard_t bb() { return 1ul << value_; }

        inline constexpr uint8_t start_of_row() { return value_ - column(); }

        static inline constexpr square index_at(uint8_t row, uint8_t col) {
            return col + 8 * row;
        }
    };

    struct bitboard : public LiteralWrapper<bitboard_t>,
                      public OperatorWrapper<bitboard> {
      public:
        constexpr bitboard(bitboard_t value = 0)
            : LiteralWrapper<bitboard_t>(value) {}

        static inline constexpr bitboard bit_at(square index) {
            return 1ul << index;
        }

        inline constexpr bool is_set_at(square index) {
            return (value_ >> index) & 1ul;
        }

        inline constexpr bitboard last_bit() { return value_ & 1; }

        inline constexpr bitboard LSB() {
            bitboard x = value_;

            return x & (-x);
        }

        inline constexpr bitboard MSB() {
            bitboard x = value_;

            x |= (x >> 1);
            x |= (x >> 2);
            x |= (x >> 4);
            x |= (x >> 8);
            x |= (x >> 16);
            x |= (x >> 32);
            x++;
            x >>= 1;

            return x;
        }

        inline constexpr uint8_t popcount() { return std::popcount(value_); }
        inline constexpr uint8_t rzeros() { return std::countr_zero(value_); }

        inline constexpr bitboard mask(bitboard mask) { return value_ & mask; }
        inline constexpr bitboard pop(bitboard mask) { return value_ & ~mask; }
        inline constexpr bitboard join(bitboard mask) { return value_ | mask; }

        inline constexpr bitboard forward(color_t color, auto shift) {
            return color ? value_ << shift : value_ >> shift;
        }

        inline constexpr bitboard backward(color_t color, auto shift) {
            return color ? value_ >> shift : value_ << shift;
        }

        /*
         * Makes a ray from a bit to the next set blocker
         */
        inline constexpr bitboard ray_next(bitboard blockers) {
            return (blockers - value_) ^ blockers;
        }

        inline constexpr bitboard flip_vertically() {
            auto value = value_;

            const uint64_t k1 = 0x00FF00FF00FF00FFul;
            const uint64_t k2 = 0x0000FFFF0000FFFFul;
            value = ((value >> 8) & k1) | ((value & k1) << 8);
            value = ((value >> 16) & k2) | ((value & k2) << 16);
            value = (value >> 32) | (value << 32);
            return value;
        }

        inline constexpr bitboard flip_horizontally() {
            auto value = value_;

            const uint64_t k1 = 0x5555555555555555ul;
            const uint64_t k2 = 0x3333333333333333ul;
            const uint64_t k4 = 0x0f0f0f0f0f0f0f0ful;
            value = ((value >> 1) & k1) + 2 * (value & k1);
            value = ((value >> 2) & k2) + 4 * (value & k2);
            value = ((value >> 4) & k4) + 16 * (value & k4);
            return value;
        }

        /*
         * Takes two bits and returns the set of bits between them (inclusive)
         * */
        static inline constexpr bitboard interval(bitboard first,
                                                  bitboard second) {
            return ((first - 1) ^ (second - 1)) | first | second;
        }

        static inline void scan(bitboard bitboard,
                                const std::function<void(square)>& processor) {
            for (square index = 0; bitboard != 0 && index < 64;
                 ++index, bitboard >>= 1) {

                if (bitboard.last_bit()) {
                    processor(index);
                }
            }
        }

        static void print(bitboard bboard) {
            for (int rank = 7; rank >= 0; --rank) {
                for (int file = 7; file >= 0; --file) {
                    int square_index = rank * 8 + file;
                    std::cout << ((bboard >> square_index) & 1ULL ? '1' : '0')
                              << " ";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
        }

        struct Masks {
            static constexpr bitboard_t fullboard = 0xFFFFFFFFFFFFFFFF;
            static constexpr bitboard_t border = 0xFF818181818181FF;

            static constexpr bitboard_t vertical = 0x0101010101010101;
            static constexpr bitboard_t horizontal = 0xFF;

            static constexpr bitboard_t diagonal_rightleft = 0x0102040810204080;
            static constexpr bitboard_t diagonal_leftright = 0x8040201008040201;

            static constexpr bitboard_t rel_blockers_vertical =
                vertical & ~0x0100000000000001;
            static constexpr bitboard_t rel_blockers_horizontal =
                horizontal & ~0x81;

            static constexpr bitboard_t diagonals[]{
                0x0000000000000001, // 0
                0x0000000000000102, // 1
                0x0000000000010204, // 2
                0x0000000001020408, // 3
                0x0000000102040810, // 4
                0x0000010204081020, // 5
                0x0001020408102040, // 6
                /*---------------*/
                0x0102040810204080, // 7
                /*---------------*/
                0x0204081020408000, // 8
                0x0408102040800000, // 9
                0x0810204080000000, // 10
                0x1020408000000000, // 11
                0x2040800000000000, // 12
                0x4080000000000000, // 13
                0x8000000000000000, // 14
            };

            static inline constexpr bitboard get_diagonal_at(square index) {
                return diagonals[index.column() + index.row()];
            }

            static inline constexpr bitboard get_rev_diagonal_at(square index) {
                return get_diagonal_at(index.start_of_row() +
                                       (7 - index.column()))
                    .flip_horizontally();
            }

            static inline constexpr bitboard make_n_mask(square index) {
                return fullboard
                       // Moves the mask one above the square
                       << (index.start_of_row() + 8);
            }

            static inline constexpr bitboard make_s_mask(square index) {
                // Moves one row above the square
                // and gets the trailing zeros
                return bitboard::bit_at(index.start_of_row() + 8) - 1;
            }

            static inline constexpr bitboard make_e_mask(square index) {
                return (bitboard::bit_at(index.column()) - 1) *
                       0x0101010101010101UL;
            }

            static inline constexpr bitboard make_w_mask(square index) {
                return (~make_e_mask(index)) ^
                       bitboard::bit_at(index) * 0x0101010101010101UL;
            }
        };
    };

    struct Color : public LiteralWrapper<color_t> {
      public:
        static constexpr color_t BLACK = false, WHITE = true;

        constexpr Color(color_t color = WHITE)
            : LiteralWrapper<color_t>(color) {}

        static constexpr color_t Both[]{BLACK, WHITE};

        inline constexpr bool isWhite() const { return value_; }
        inline constexpr bool isBlack() const { return !value_; }
    };

    struct Piece : public LiteralWrapper<piece_t> {
      public:
        static constexpr piece_t PAWNS = 0, KNIGHTS = 1, BISHOPS = 2, ROOKS = 3,
                                 QUEENS = 4, KINGS = 5, NONE = 6;

        constexpr Piece(piece_t piece) : LiteralWrapper<piece_t>(piece) {}

        static constexpr piece_t All[]{PAWNS, KNIGHTS, BISHOPS,
                                       ROOKS, QUEENS,  KINGS};

        static Piece char_to_piece(char c);
        static char piece_to_char(Piece piece, Color color);

        inline constexpr bool isPawn() const { return value_ == PAWNS; }
        inline constexpr bool isKnight() const { return value_ == KNIGHTS; }
        inline constexpr bool isBishop() const { return value_ == BISHOPS; }
        inline constexpr bool isRook() const { return value_ == ROOKS; }
        inline constexpr bool isQueen() const { return value_ == QUEENS; }
        inline constexpr bool isKing() const { return value_ == KINGS; }
        inline constexpr bool isNone() const { return value_ == NONE; }
    };

    struct Direction : public LiteralWrapper<direction_t> {
      public:
        static constexpr direction_t WEST = true, EAST = false;

        constexpr Direction(direction_t direction)
            : LiteralWrapper<direction_t>(direction) {}

        inline constexpr bool isWest() { return value_; }
        inline constexpr bool isEast() { return !value_; }
    };

} // namespace Game
