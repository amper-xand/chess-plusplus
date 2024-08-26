#pragma once

#include <cstdint>
#include <string>

namespace Types {
    typedef uint64_t bitboard;
    typedef uint8_t square;

} // namespace Types

namespace Game::Colors {
    enum Color { BLACK = false, WHITE = true };
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

    // clang-format off
    struct Move {
        square        from, to;
        Pieces::Piece piece_moved, piece_captured = Pieces::NONE;
    };
    // clang-format on

    struct Board {
        // clang-format off
        Colors::Color turn;
        square        enpassant;

        union { bitboard colors[2]; struct { bitboard black, white; }; };

        union { bitboard pieces[6]; struct { bitboard pawns, knights, bishops, rooks, queens, kings; }; };
        // clang-format on

        static Board parse_fen_string(std::string fen);

        void print_board();

        bool is_square_occupied(square index);

        Colors::Color color_at(square index);

        Pieces::Piece piece_at(square index);

        inline bitboard all_pieces() { return white | black; }

        void make_move(Move move);
    };

} // namespace Game
