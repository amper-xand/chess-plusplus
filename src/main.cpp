#include <cstdio>
#include <pthread.h>

#include "core/game.hpp"
#include "core/generators/generators.hpp"

int main() {
    Game::Generators::initialize_tables();

    auto board = Game::Board::parse_fen_string(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    board.print_board();
    auto moves = Game::Generators::generate_moves(board);
    board.make_move(moves.at(17));
    board.print_board();

    return 0;
}
