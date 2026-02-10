#include <print>

#include <core/generation.hpp>
#include <core/notation.hpp>
#include <core/types.hpp>

int main() {
    std::print("Hello world\n");

    auto fen = core::notation::FEN::parse_string(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    auto board = fen.get_board();

    auto moves = core::generation::generate_moves(board);

    std::print("{}", core::notation::draw_board_ascii(board));
}
