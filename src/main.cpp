#include <cstdio>
#include <pthread.h>

#include "core/generators/generators.hpp"

int main() {
    Game::Generators::initialize_tables();

    auto board = Game::Board::from_fen(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    board.print();
    auto moves = Game::Generators::generate_moves(board);
    board.play(moves.at(17));
    board.print();

    return 0;
}
