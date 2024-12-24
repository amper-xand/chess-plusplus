#pragma once

#include <core/representation.hpp>

#include <array>
#include <vector>

namespace core::generators {

    constexpr uint8_t max_moves = 218;

    struct MoveGenerator {
        // clang-format off
      private:
        std::array<Move, max_moves> moves;
        std::array<Move, max_moves>::iterator next_move = moves.begin();

      public:
        Board board;

        struct { bitboard absolute, partial, pinners; } pins;

        struct { bool pinned = false; } enpassant;
        // clang-format on

        MoveGenerator(Board board);

        Move& next();

        void from_bitboard(Piece piece, square from, bitboard moves,
                           bitboard captures, Move base = Move());

        std::vector<Move> get_generated();
    };

    void initialize_tables();

    std::vector<Move> generate_moves(Board board);
} // namespace Game::Generators
