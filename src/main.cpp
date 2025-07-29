#include <print>

#include <core/generation.hpp>
#include <core/types.hpp>

int main() {
    std::print("Hello world\n");

    auto board = core::Board::parse_fen_repr(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    auto moves = core::generation::generate_moves(board);

    board.print();
}
