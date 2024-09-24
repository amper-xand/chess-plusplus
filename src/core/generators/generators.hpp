#pragma once

#include "../game.hpp"

#include <array>
#include <span>
#include <vector>

namespace Game::Generators {

    constexpr uint8_t max_moves = 218;

    struct MoveGenerator {
        // clang-format off
      private:
        std::array<Move, max_moves> moves;
        std::array<Move, max_moves>::iterator next_move = moves.begin();

      public:
        Board board;

        bitboard pinned;

        struct { bool pinned = false; } enpassant;
        // clang-format on

        MoveGenerator(Board board);

        std::span<Move> next_n(int n);

        Move& next();

        std::vector<Move> get_generated();
    };

    void initialize_tables();

    std::vector<Move> generate_moves(Board board);
} // namespace Game::Generators
