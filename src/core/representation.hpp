#pragma once

#include <string>

#include "types.hpp"

namespace core {

struct Move {
    square from;
    square to;

    Piece moved = Piece::NONE;
    Piece target = Piece::NONE;
};

struct Board {
    // clang-format off

        union  { bitboard colors[2]; 
        struct { bitboard_t black, white; }; };

        union  { bitboard pieces[6];
        struct { bitboard_t pawns, knights, bishops, rooks, queens, kings; }; };

        Color turn;

    // clang-format on

    Board() : colors{bitboard(0)}, pieces{bitboard(0)} {}

    void play(const Move move);
    void unplay(const Move move);

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

    constexpr inline bool operator==(const Board& other) const {
        for (auto piece : Piece::All) {
            if (this->pieces[piece] != other.pieces[piece])
                return false;
        }

        if (this->white != other.white)
            return false;

        if (this->black != other.black)
            return false;

        return true;
    }

    static Board parse_fen_repr(std::string fen);

    void print();
};
}  // namespace core
