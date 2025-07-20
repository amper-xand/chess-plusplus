#pragma once

#include <core/representation.hpp>

#include <array>
#include <vector>

namespace core::generation {

constexpr uint8_t max_moves = 218;

class GenerationContext {
   private:
    std::array<Move, max_moves> moves;
    uint8_t nxt = 0;

   public:
    const Board& board;

    GenerationContext(const Board& board) : board(board) {}

    Move& next();

    void bulk(Piece moved, square from, bitboard moves, bitboard capturable);

    std::vector<Move> get_generated_moves();
};

std::vector<Move> generate_moves(const Board& board);

void generate_pawn_moves(GenerationContext& context);

void generate_knight_moves(GenerationContext& context);

void generate_rook_moves(generation::GenerationContext& context);

void generate_bishop_moves(generation::GenerationContext& context);

void generate_queen_moves(generation::GenerationContext& context);

void generate_king_moves(generation::GenerationContext& context);

}  // namespace core::generation
