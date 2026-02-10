#pragma once

#include <stdint.h>

namespace core {

typedef uint8_t square_t;
typedef uint64_t bitboard_t;

typedef uint8_t color_t;
typedef uint8_t piece_t;

/**
 * @brief Wraps a literal or primitive type
 * to allow extended behavior
 * while retaining implicit conversions.
 *
 * @tparam type The underlying type to wrap.
 *
 * This class allows derived classes to behave like the wrapped type while
 * adding utility methods.
 * Implicit conversions and pointer access operators
 * are forwarded to the underlying value.
 * Gets optimized out as wrapped type.
 * */
template <typename type>
class LiteralWrapper {
   protected:
    type value_ = 0;

   public:
    constexpr LiteralWrapper(type value) : value_(value) {}

    constexpr operator type &() { return value_; }
    constexpr operator const type &() const { return value_; }

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

/*
 * @brief Represents the board as a set of bits.
 *
 * Bit mapping is as follows
 *
 *  v- 63              index
 * A8 0 0 0 0 0 0 H8 <- 56
 *  0 0 0 0 0 0 0 0  <- 48
 *  0 0 0 0 0 0 0 0  <- 40
 *  0 0 0 0 0 0 0 0  <- 32
 *  0 0 0 0 0 0 0 0  <- 24
 *  0 0 0 0 0 0 0 0  <- 16
 *  0 0 0 0 0 0 0 0  <- 8
 * A1 0 0 0 0 0 0 H1 <- 0
 * */
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
         * Takes two bits and returns the set of bits between them (inclusive).
         * 0 is interpreted as the 65th bit
         * */
        static inline constexpr bitboard interval(
            bitboard first, bitboard second) {
            return ((first - 1) ^ (second - 1)) | first | second;
        }
    };
};

#ifndef CORE_PRIMITIVES_ONLY

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

struct Move {
    square from;
    square to;

    Piece moved = Piece::NONE;
    Piece target = Piece::NONE;

    Piece promotion = Piece::NONE;

    // clang-format off

    //
    bool en_passant : 1 = 0; // set when (enabling ep) or (taking ep)
    bool castle     : 1 = 0; // set when castling
    bool check      : 1 = 0; // set when moving piece causes check
    bool mate       : 1 = 0; // set when check causes mate

    // clang-format on
};

/*
 * Represents game state
 * */
struct Board {
    // clang-format off

    union  { bitboard colors[2]; 
    struct { bitboard_t black, white; }; };

    union  { bitboard pieces[6];
    struct { bitboard_t pawns, knights, bishops, rooks, queens, kings; }; };

    Color turn;

    struct State {
        struct Castling {
            bool white_left;
            bool white_right;
            bool black_left;
            bool black_right;
        };

        State::Castling castling{false, false, false, false};
        square en_passant = 65;
        uint8_t fifty_move_rule_counter = 0;
    };

    union {
        State state;
        struct {
            State::Castling castling{false, false, false, false};
            square_t en_passant = 65;
            uint8_t fifty_move_rule_counter = 0;
        };
    };

    // clang-format on

    inline bool get_castling_left() const {
        return turn ? castling.white_left : castling.black_left;
    }

    inline bool get_castling_right() const {
        return turn ? castling.white_right : castling.black_right;
    }

    inline void set_castling_left(bool value) {
        (turn ? castling.white_left : castling.black_left) = value;
    }

    inline void set_castling_right(bool value) {
        (turn ? castling.white_right : castling.black_right) = value;
    }

    Board() : colors{bitboard(0)}, pieces{bitboard(0)} {}

    const State play(const Move move);
    void unplay(const Move move, const State prev);

    inline bitboard all() const { return white | black; }

    inline bitboard allies() const { return colors[turn]; }

    inline bitboard allied(Piece piece) const {
        return pieces[piece] & colors[turn];
    }

    inline bitboard enemies() const { return colors[!turn]; }

    inline bitboard enemy(Piece piece) const {
        return pieces[piece] & colors[!turn];
    }

    inline bool occupied(square index) const {
        return bitboard(white | black)[index];
    }

    inline Color color(square index) const {
        return Color::Both[bitboard(white)[index]];
    }

    inline Piece piece(square index) const {
        for (auto piece : Piece::All) {
            if (pieces[piece][index]) return piece;
        }

        return Piece::NONE;
    }

    constexpr inline bool operator==(const Board &other) const {
        for (auto piece : Piece::All) {
            if (this->pieces[piece] != other.pieces[piece]) return false;
        }

        if (this->white != other.white) return false;

        if (this->black != other.black) return false;

        if (this->turn != other.turn) return false;

        if (this->state.fifty_move_rule_counter !=
            other.state.fifty_move_rule_counter)
            return false;

        if (this->state.en_passant !=  //
            other.state.en_passant)
            return false;

        if (this->state.castling.white_left !=  //
            other.state.castling.white_left)
            return false;

        if (this->state.castling.black_left !=  //
            other.state.castling.black_left)
            return false;

        if (this->state.castling.white_right !=
            other.state.castling.white_right)
            return false;

        if (this->state.castling.black_right !=
            other.state.castling.black_right)
            return false;

        return true;
    }

    static Board parse_fen_repr(std::string fen);

    void print();
};

#else
#undef CORE_PRIMITIVES_ONLY
#endif

}  // namespace core
