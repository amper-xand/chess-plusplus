#include <core/generation.hpp>
#include <core/representation.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <format>
#include <string>

namespace core::test {

struct MoveCounts {
    union {
        uint16_t pieces[6];
        struct {
            uint16_t pawns, knights, bishops, rooks, queens, kings;
        };
    };
};

struct PositionTestCase {
    std::string fen;
    MoveCounts moves;
};

static const PositionTestCase kMoveGenerationTestCases[] = {
    {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR",
        {.pawns = 16, .knights = 4, .rooks = 0}},

    // Pawn-only or pawn-heavy positions
    {"8/8/8/3pP3/8/8/8/8 w", {.pawns = 1, .knights = 0, .rooks = 0}},
    {"8/2p5/8/8/8/8/2P5/8 w", {.pawns = 2, .knights = 0, .rooks = 0}},
    {"8/8/3p4/2pPp3/8/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 0}},
    {"8/8/8/3P4/3p4/8/8/8 w", {.pawns = 1, .knights = 0, .rooks = 0}},
    {"8/8/8/8/2P1P1P1/8/2P1P1P1/8 w", {.pawns = 6, .knights = 0, .rooks = 0}},
    {"8/8/8/2p1p1p1/2P1P1P1/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 0}},
    {"8/8/8/3P4/3P4/3P4/3P4/8 w", {.pawns = 1, .knights = 0, .rooks = 0}},
    {"8/8/8/8/8/8/PPPPPPPP/8 w", {.pawns = 16, .knights = 0, .rooks = 0}},
    {"8/2p1p1p1/8/8/8/8/2P1P1P1/8 w", {.pawns = 6, .knights = 0, .rooks = 0}},

    // Knight-only or knight-heavy positions
    {"8/8/8/8/8/8/8/N7 w", {.pawns = 0, .knights = 2, .rooks = 0}},
    {"8/8/8/8/8/8/8/NN6 w", {.pawns = 0, .knights = 5, .rooks = 0}},
    {"8/8/8/3N4/8/8/8/8 w", {.pawns = 0, .knights = 8, .rooks = 0}},
    {"8/8/3n4/8/3N4/8/8/8 w", {.pawns = 0, .knights = 8, .rooks = 0}},

    // Rook-only or rook-heavy positions
    {"8/8/8/8/8/8/8/R7 w", {.pawns = 0, .knights = 0, .rooks = 14}},
    {"8/8/8/8/8/8/8/RR6 w", {.pawns = 0, .knights = 0, .rooks = 20}},
    {"8/8/8/3R4/8/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 14}},
    {"8/8/3r4/8/3R4/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 12}},

    // Blocked rooks
    {"8/8/8/8/8/8/p7/R7 w", {.pawns = 0, .knights = 0, .rooks = 8}},
    {"8/8/8/8/8/8/7p/7R w", {.pawns = 0, .knights = 0, .rooks = 8}},
    {"R7/p7/8/8/8/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 8}},
    {"7R/7p/8/8/8/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 8}},
    {"8/8/8/8/8/8/8/Rp6 w", {.pawns = 0, .knights = 0, .rooks = 8}},
    {"8/8/8/8/8/8/8/6pR w", {.pawns = 0, .knights = 0, .rooks = 8}},
    {"Rp6/8/8/8/8/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 8}},
    {"6pR/8/8/8/8/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 8}},

    {"8/8/8/8/8/8/8/Rp1p4 w", {.pawns = 0, .knights = 0, .rooks = 8}},
    {"8/8/8/8/8/8/8/1pR5 w", {.pawns = 0, .knights = 0, .rooks = 13}},
    {"8/8/8/8/8/8/8/2R1p3 w", {.pawns = 0, .knights = 0, .rooks = 11}},

    {"8/8/3p4/2pRp3/3p4/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 4}},
    {"8/8/3p4/2pRp3/8/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 7}},
    {"8/8/3p4/2pR4/3p4/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 7}},
    {"8/8/3p4/3Rp3/3p4/8/8/8 w", {.pawns = 0, .knights = 0, .rooks = 6}},
    {"8/8/8/2pRp3/3p4/8/8/8 w ", {.pawns = 0, .knights = 0, .rooks = 6}},

};

class MoveGenerationTest : public ::testing::TestWithParam<PositionTestCase> {};

TEST_P(MoveGenerationTest, MatchesExpectedMoveCounts) {
    const auto& test_case = GetParam();

    auto board = Board::parse_fen_repr(test_case.fen);
    auto moves = generation::generate_moves(board);

    for (Piece piece : {Piece::PAWNS, Piece::KNIGHTS, Piece::ROOKS}) {
        int produced_count = std::ranges::count_if(
            moves, [&](Move move) { return move.moved == piece; });

        std::string piece_name = [&]() {
            if (piece == Piece::PAWNS) return "pawn";
            if (piece == Piece::KNIGHTS) return "knight";
            if (piece == Piece::ROOKS) return "rook";

            return "[[INVALID]]";
        }();

        EXPECT_EQ(produced_count, test_case.moves.pieces[piece])
            << std::format("FEN: {}\n", test_case.fen)
            << std::format("Expected {} moves: {}, got: {}", piece_name,
                   test_case.moves.pieces[piece], produced_count);
    }
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
