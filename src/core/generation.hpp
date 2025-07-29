#pragma once

#include <core/types.hpp>

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

    struct {
        bitboard absolute = 0, partial = 0;
    } pinned;

    bitboard attacked_squares = 0;

    GenerationContext(const Board& board) : board(board) {}

    Move& next();

    void bulk(Piece moved, square from, bitboard moves, bitboard capturable);

    std::vector<Move> get_generated_moves();
};

std::vector<Move> generate_moves(const Board& board);

void generate_moves_pawn(GenerationContext& context);

void generate_moves_knight(GenerationContext& context);

void generate_moves_rook(GenerationContext& context);

void generate_moves_bishop(GenerationContext& context);

void generate_moves_queen(GenerationContext& context);

void generate_moves_king(GenerationContext& context);

bitboard generate_bitboard_squares_attacked(GenerationContext& context);

void generate_bitboard_pieces_pinned(
    GenerationContext& context, bitboard& absolute, bitboard& partial);

}  // namespace core::generation
