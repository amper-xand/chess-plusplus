#pragma once

#include <core/representation.hpp>

#include <array>
#include <vector>

namespace core {

constexpr uint8_t max_moves = 218;

class MoveGenerator {
   private:
    std::array<Move, max_moves> moves;
    uint8_t nxt = 0;
    const Board& board;

   public:
    static std::vector<Move> generate_moves(const Board& board);

   protected:
    MoveGenerator(const Board& board) : board(board) {}

    std::vector<Move> generate_moves();

    void generate_pawn_moves();

   private:
    Move& next();
};

}  // namespace core
