#include <cstdio>
#include <pthread.h>

#include <core/generators.hpp>

int main() {
    core::generators::initialize_tables();

    auto board = core::Board::parse_fen(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    board.print();
    auto moves = core::generators::generate_moves(board);
    board.play(moves.at(17));
    board.print();

    return 0;
}
