#include <print>

#include "core/representation.hpp"

int main() {
    std::print("Hello world\n");

    auto board = core::Board::parse_fen_repr(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    board.print();
}
