#include <core/generation.hpp>
#include <core/representation.hpp>

#include <algorithm>
#include <format>
#include <print>
#include <ranges>
#include <string>

namespace core::test {

std::string message(std::string title, std::string msg, auto expected,
    auto produced, bool passed) {
    constexpr int width = 40;

    // Format title line with centered title
    std::string base =
        std::format("{:=^{}}\n", std::format("==[ {} ]==", title), width);

    std::string status =
        std::format("{}\n", passed ? "✔ Test Passed" : "✗ Test Failed");

    std::string user_msg = msg.empty() ? "" : std::format("Info: {}\n", msg);

    std::string details =
        std::format("Expected:\n{}\nProduced:\n{}\n", expected, produced);

    std::string end =
        std::format("{:=^{}}\n", "", width);  // Bottom bar same width

    return base + status + user_msg + details + end;
}

void TestGenerationCount(bool pass_silently) {
    struct MoveCounts {
        uint8_t pawns = 0;
    };

    struct PositionTestCase {
        std::string fen;
        MoveCounts moves;
    };

    static const PositionTestCase cases[] = {
        {.fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
            .moves = {.pawns = 16}},
        {.fen = "8/8/8/3pP3/8/8/8/8 w", .moves = {.pawns = 1}},
        {.fen = "8/2p5/8/8/8/8/2P5/8 w", .moves = {.pawns = 2}},
        {.fen = "8/8/3p4/2pPp3/8/8/8/8 w", .moves = {.pawns = 0}},
        {.fen = "8/8/8/3P4/3p4/8/8/8 w", .moves = {.pawns = 1}},
        {.fen = "8/8/8/8/2P1P1P1/8/2P1P1P1/8 w", .moves = {.pawns = 6}},
        {.fen = "8/8/8/2p1p1p1/2P1P1P1/8/8/8 w", .moves = {.pawns = 0}},
        {.fen = "8/8/8/3P4/3P4/3P4/3P4/8 w", .moves = {.pawns = 1}},
        {.fen = "8/8/8/8/8/8/PPPPPPPP/8 w", .moves = {.pawns = 16}},
        {.fen = "8/2p1p1p1/8/8/8/8/2P1P1P1/8 w", .moves = {.pawns = 6}}};

    // initialize boards
    auto boards = cases |
        std::ranges::views::transform(
            [&](const PositionTestCase& position_case) {
                return Board::parse_fen_repr(position_case.fen);
            });

    // generate moves for every position
    auto generation_results = boards |
        std::ranges::views::transform(
            [&](auto board) { return MoveGenerator::generate_moves(board); });

    // test generation for each position
    for (auto [moves, position_case] :
        std::ranges::zip_view(generation_results, cases)) {
        MoveCounts result_count;

        result_count.pawns = std::ranges::count_if(
            moves, [](Move move) { return move.moved == Piece::PAWNS; });

        bool passed = position_case.moves.pawns == result_count.pawns;

        if (passed && pass_silently) continue;

        std::println("{}",
            message("Pawn Count Generation Test",
                std::format("FEN: {}", position_case.fen),
                position_case.moves.pawns, result_count.pawns, passed));
    }
}

}  // namespace core::test

int main(int argc, char* argv[]) {
    bool pass_silenty = false;

    for (int i = 0; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--pass-silently") pass_silenty = true;
    }

    core::test::TestGenerationCount(pass_silenty);
    return 0;
}
