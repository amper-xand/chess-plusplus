#include <print>

#include <core/representation.hpp>
#include <core/generation.hpp>

int main() {
    std::print("Hello world\n");

    auto board = core::Board::parse_fen_repr(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    auto moves = core::MoveGenerator::generate_moves(board);

    board.print();
}
