#pragma once

#include <cstdint>
#include <string>

namespace Types {
    typedef uint64_t bitboard;
    typedef uint8_t square;

    inline bitboard get_bit_set_at(square index) { return 1ul << index; }

    inline bool is_set_at(square index, bitboard bitboard) {
        return (bitboard >> index) & 1ul;
    }

} // namespace Types

namespace Game::Colors {
    enum Color { BLACK, WHITE };
    static const Color BothColors[]{BLACK, WHITE};

} // namespace Game::Colors

namespace Game::Pieces {
    enum Piece { PAWNS, KNIGHTS, BISHOPS, ROOKS, QUEENS, KINGS, NONE };
    static const Piece AllPieces[] = {PAWNS, KNIGHTS, BISHOPS,
                                      ROOKS, QUEENS,  KINGS};

    Piece char_to_piece(char c);

    char piece_to_char(Piece piece, Colors::Color color);

} // namespace Game::Pieces

namespace Game {
    using namespace Types;

    struct Move {
        square from, to;
        Pieces::Piece moved;
        Colors::Color color;
    };

    struct Board {
        union {
            bitboard colors[2];
            struct {
                bitboard black, white;
            };
        };

        union {
            bitboard pieces[8];
            struct {
                bitboard pawns, knights, bishops, rooks, queens, kings;
            };
        };

        square enpassant;

        static Board parse_fen_string(std::string fen);

        void print_board();

        bool is_square_occupied(square index);

        Colors::Color color_at(square index);

        Pieces::Piece piece_at(square index);

        void make_move(Move move);
    };

} // namespace Game
