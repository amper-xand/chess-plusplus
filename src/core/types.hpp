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

    inline constexpr uint8_t column() { return value_ % 8; }

    inline constexpr uint8_t row() { return value_ / 8; }

    inline constexpr square right(uint8_t v = 1) { return value_ - v; }
    inline constexpr square left(uint8_t v = 1) { return value_ + v; }
    inline constexpr square up(uint8_t v = 1) { return value_ + 8 * v; }
    inline constexpr square down(uint8_t v = 1) { return value_ - 8 * v; }

    inline constexpr bitboard_t bb() { return 1ul << value_; }

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

    inline constexpr bitboard mask(bitboard mask) { return value_ & mask; }
    inline constexpr bitboard exclude(bitboard mask) { return value_ & ~mask; }
    inline constexpr bitboard join(bitboard mask) { return value_ | mask; }

    inline constexpr bitboard forward(color_t color, auto shift = 1) {
        return color ? value_ << shift : value_ >> shift;
    }

    inline constexpr bitboard backward(color_t color, auto shift = 1) {
        return color ? value_ >> shift : value_ << shift;
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

    static constexpr piece_t All[]{PAWNS, KNIGHTS, BISHOPS,
                                   ROOKS, QUEENS,  KINGS};

    static Piece from_character_repr(char c);
    char to_character_repr(Color color);

    inline constexpr bool isPawn() const { return value_ == PAWNS; }
    inline constexpr bool isKnight() const { return value_ == KNIGHTS; }
    inline constexpr bool isBishop() const { return value_ == BISHOPS; }
    inline constexpr bool isRook() const { return value_ == ROOKS; }
    inline constexpr bool isQueen() const { return value_ == QUEENS; }
    inline constexpr bool isKing() const { return value_ == KINGS; }
    inline constexpr bool isNone() const { return value_ == NONE; }
};
}  // namespace core
