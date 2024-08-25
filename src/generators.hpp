#pragma once

#include "game.hpp"
#include <vector>

namespace Game::Generators {
    std::vector<Move> gen_pawn_moves(Game::Board board);

    std::vector<Move> gen_rooks_moves(Game::Board board);

    std::vector<Move> gen_bishops_moves(Game::Board board);

    std::vector<Move> gen_queens_moves(Game::Board board);
} // namespace Game::Generators
