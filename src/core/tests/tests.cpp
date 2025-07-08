#include <core/generation.hpp>
#include <core/representation.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <format>
#include <string>

namespace core::test {

struct MoveCounts {
    uint8_t pawns = 0;
    uint8_t knights = 0;
};

struct PositionTestCase {
    std::string fen;
    MoveCounts moves;
};

static const PositionTestCase kMoveGenerationTestCases[] = {
    {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
        {.pawns = 16, .knights = 4}},

    // Pawn-only or pawn-heavy positions
    {"8/8/8/3pP3/8/8/8/8 w", {.pawns = 1, .knights = 0}},
    {"8/2p5/8/8/8/8/2P5/8 w", {.pawns = 2, .knights = 0}},
    {"8/8/3p4/2pPp3/8/8/8/8 w", {.pawns = 0, .knights = 0}},
    {"8/8/8/3P4/3p4/8/8/8 w", {.pawns = 1, .knights = 0}},
    {"8/8/8/8/2P1P1P1/8/2P1P1P1/8 w", {.pawns = 6, .knights = 0}},
    {"8/8/8/2p1p1p1/2P1P1P1/8/8/8 w", {.pawns = 0, .knights = 0}},
    {"8/8/8/3P4/3P4/3P4/3P4/8 w", {.pawns = 1, .knights = 0}},
    {"8/8/8/8/8/8/PPPPPPPP/8 w", {.pawns = 16, .knights = 0}},
    {"8/2p1p1p1/8/8/8/8/2P1P1P1/8 w", {.pawns = 6, .knights = 0}},

    // Knight-only or knight-heavy positions
    {"8/8/8/8/8/8/8/N7 w", {.pawns = 0, .knights = 2}},
    {"8/8/8/8/8/8/8/NN6 w", {.pawns = 0, .knights = 5}},
    {"8/8/8/3N4/8/8/8/8 w", {.pawns = 0, .knights = 8}},
    {"8/8/3n4/8/3N4/8/8/8 w", {.pawns = 0, .knights = 8}},
};

class MoveGenerationTest : public ::testing::TestWithParam<PositionTestCase> {};

TEST_P(MoveGenerationTest, MatchesExpectedMoveCounts) {
    const auto& test_case = GetParam();

    auto board = Board::parse_fen_repr(test_case.fen);
    auto moves = generation::generate_moves(board);

    int actual_pawn_moves = std::ranges::count_if(
        moves, [](Move move) { return move.moved == Piece::PAWNS; });

    int actual_knight_moves = std::ranges::count_if(
        moves, [](Move move) { return move.moved == Piece::KNIGHTS; });

    EXPECT_EQ(actual_pawn_moves, test_case.moves.pawns)
        << std::format("FEN: {}\nExpected pawn moves: {}, got: {}",
               test_case.fen, test_case.moves.pawns, actual_pawn_moves);

    EXPECT_EQ(actual_knight_moves, test_case.moves.knights)
        << std::format("FEN: {}\nExpected knight moves: {}, got: {}",
               test_case.fen, test_case.moves.knights, actual_knight_moves);
}

INSTANTIATE_TEST_SUITE_P(AllPositions, MoveGenerationTest,
    ::testing::ValuesIn(kMoveGenerationTestCases));

constexpr int kDepth = 3;

void test_reversible_move_sequence(Board& board, int depth) {
    if (depth == 0) return;

    const Board original = board;

    auto moves = generation::generate_moves(board);

    for (const Move& m : moves) {
        board.play(m);
        board.unplay(m);

        ASSERT_EQ(board, original) << std::format(
            "Board mismatch after unmaking move at depth {}\n", depth
            // TODO: "Move: {}\nFEN: {}\n",
            // TODO: , move.to_string(), Board::to_fen_repr(original)
        );
    }

    // if the position can be reversed properly
    // check its branches

    for (const Move& m : moves) {
        board.play(m);
        test_reversible_move_sequence(board, depth - 1);
        board.unplay(m);
    }
}

TEST(FENMoveUnmoveTest, PlaysAndUnplaysWithoutCorruption) {
    std::string fen =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Board board = Board::parse_fen_repr(fen);

    EXPECT_NO_FATAL_FAILURE({ test_reversible_move_sequence(board, kDepth); });
}

}  // namespace core::test

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
