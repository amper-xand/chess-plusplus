#include <cstdio>
#include <pthread.h>

#include "game.hpp"

int main() {
    auto board = Game::Board::parse_fen_string(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    board.print_board();
    board.make_move(Game::Move{8, 16, Game::Pieces::PAWNS, Game::Colors::WHITE});
    board.print_board();

    return 0;
}
