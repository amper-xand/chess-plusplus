#pragma once

#include <stdint.h>

namespace core {
typedef uint8_t square_t;
typedef uint64_t bitboard_t;

typedef uint8_t color_t;
typedef uint8_t piece_t;

template <typename type>
class LiteralWrapper {
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

struct square : public LiteralWrapper<square_t> {
   public:
    constexpr square(square_t value = 0) : LiteralWrapper<square_t>(value) {}

    inline constexpr uint8_t column() const { return value_ % 8; }

    inline constexpr uint8_t row() const { return value_ / 8; }

    inline constexpr square right(uint8_t v = 1) const { return value_ - v; }
    inline constexpr square left(uint8_t v = 1) const { return value_ + v; }
    inline constexpr square up(uint8_t v = 1) const { return value_ + 8 * v; }
    inline constexpr square down(uint8_t v = 1) const { return value_ - 8 * v; }

    inline constexpr bitboard_t bb() const { return 1ul << value_; }

    static inline constexpr square at(uint8_t row, uint8_t col) {
        return col + 8 * row;
    }
};

struct bitboard : public LiteralWrapper<bitboard_t> {
   public:
    constexpr bitboard(bitboard_t value = 0)
        : LiteralWrapper<bitboard_t>(value) {}

    inline constexpr bool operator[](square index) const {
        return (value_ >> index) & 1ul;
    }

    inline constexpr bitboard mask(bitboard mask) const {
        return value_ & mask;
    }
    inline constexpr bitboard exclude(bitboard mask) const {
        return value_ & ~mask;
    }
    inline constexpr bitboard join(bitboard mask) const {
        return value_ | mask;
    }

    inline constexpr bitboard forward(color_t color, auto shift = 1) const {
        return color ? value_ << shift : value_ >> shift;
    }

    inline constexpr bitboard backward(color_t color, auto shift = 1) const {
        return color ? value_ >> shift : value_ << shift;
    }

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

    struct masks {
        static constexpr bitboard_t fullboard = 0xFFFFFFFFFFFFFFFF;
        static constexpr bitboard_t border = 0xFF818181818181FF;

        static constexpr bitboard_t vertical = 0x0101010101010101;
        static constexpr bitboard_t horizontal = 0xFF;

        static inline constexpr bitboard at(square index) {
            return 1ul << index;
        }

        static inline constexpr bitboard file(uint8_t file) {
            return vertical << file;
        }

        static inline constexpr bitboard rank(uint8_t rank) {
            return horizontal << (8 * rank);
        }

        static constexpr bitboard_t diagonals[]{
            0x0000000000000001,
            0x0000000000000102,
            0x0000000000010204,
            0x0000000001020408,
            0x0000000102040810,
            0x0000010204081020,
            0x0001020408102040,
            /*---------------*/
            0x0102040810204080,
            /*---------------*/
            0x0204081020408000,
            0x0408102040800000,
            0x0810204080000000,
            0x1020408000000000,
            0x2040800000000000,
            0x4080000000000000,
            0x8000000000000000,
        };

        static constexpr bitboard_t rdiagonals[]{
            0x0100000000000000,
            0x0201000000000000,
            0x0402010000000000,
            0x0804020100000000,
            0x1008040201000000,
            0x2010080402010000,
            0x4020100804020100,
            /*---------------*/
            0x8040201008040201,
            /*---------------*/
            0x0080402010080402,
            0x0000804020100804,
            0x0000008040201008,
            0x0000000080402010,
            0x0000000000804020,
            0x0000000000008040,
            0x0000000000000080,
        };

        static inline constexpr bitboard diagonal_at(square index) {
            return diagonals[index.column() + index.row()];
        }

        static inline constexpr bitboard rev_diagonal_at(square index) {
            return rdiagonals[index.column() + (7 - index.row())];
        }

        /*
         * Takes two bits and returns the set of bits between them (inclusive)
         * */
        static inline constexpr bitboard interval(
            bitboard first, bitboard second) {
            return ((first - 1) ^ (second - 1)) | first | second;
        }
    };
};

struct Color : public LiteralWrapper<color_t> {
   public:
    static constexpr color_t BLACK = false, WHITE = true;

    constexpr Color(color_t color = WHITE) : LiteralWrapper<color_t>(color) {}

    static constexpr color_t Both[]{BLACK, WHITE};

    inline constexpr bool isWhite() const { return value_; }
    inline constexpr bool isBlack() const { return !value_; }
};

struct Piece : public LiteralWrapper<piece_t> {
   public:
    static constexpr piece_t PAWNS = 0, KNIGHTS = 1, BISHOPS = 2, ROOKS = 3,
                             QUEENS = 4, KINGS = 5, NONE = 6;

    constexpr Piece(piece_t piece) : LiteralWrapper<piece_t>(piece) {}

    static constexpr piece_t All[]{
        PAWNS, KNIGHTS, BISHOPS, ROOKS, QUEENS, KINGS};

    static Piece from_character_repr(char c);
    char to_character_repr(Color color);

    inline constexpr bool isPawn() const { return value_ == PAWNS; }
    inline constexpr bool isKnight() const { return value_ == KNIGHTS; }
    inline constexpr bool isBishop() const { return value_ == BISHOPS; }
    inline constexpr bool isRook() const { return value_ == ROOKS; }
    inline constexpr bool isQueen() const { return value_ == QUEENS; }
    inline constexpr bool isKing() const { return value_ == KINGS; }
    inline constexpr bool isNone() const { return value_ == NONE; }
    inline constexpr bool isValid() const { return value_ < NONE; }
};
}  // namespace core
