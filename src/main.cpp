#include <cstdio>
#include <pthread.h>

#include "game.hpp"
#include "generators.hpp"
#include "magic.hpp"

int main() {
    Game::Generators::Magic::initialize_magic_tables();

    auto board = Game::Board::parse_fen_string(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    board.print_board();
    board.make_move(Game::Generators::gen_rooks_moves(board).at(0));
    board.print_board();

    return 0;
}
