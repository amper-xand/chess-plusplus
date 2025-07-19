#include <core/generation.hpp>
#include <core/representation.hpp>

#include <gtest/gtest.h>
#define TOML_EXCEPTIONS 0
#include <toml++/toml.hpp>

#include <algorithm>
#include <filesystem>
#include <format>
#include <print>
#include <string>
#include <vector>

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

std::vector<PositionTestCase> MoveGenerationTestCases;

class MoveGenerationTest : public ::testing::TestWithParam<PositionTestCase> {};

TEST_P(MoveGenerationTest, MatchesExpectedMoveCounts) {
    const auto& test_case = GetParam();

    auto board = Board::parse_fen_repr(test_case.fen);
    auto moves = generation::generate_moves(board);

    for (Piece piece : {Piece::PAWNS, Piece::KNIGHTS, Piece::ROOKS, Piece::BISHOPS, Piece::QUEENS}) {
        int produced_count = std::ranges::count_if(
            moves, [&](Move move) { return move.moved == piece; });

        std::string piece_name = [&]() {
            if (piece == Piece::PAWNS) return "pawn";
            if (piece == Piece::KNIGHTS) return "knight";
            if (piece == Piece::ROOKS) return "rook";
            if (piece == Piece::BISHOPS) return "bishop";
            if (piece == Piece::QUEENS) return "queen";

            return "[[INVALID]]";
        }();

        EXPECT_EQ(produced_count, test_case.moves.pieces[piece])
            << std::format("FEN: {}\n", test_case.fen)
            << std::format("Expected {} moves: {}, got: {}", piece_name,
                   test_case.moves.pieces[piece], produced_count);
    }
}

INSTANTIATE_TEST_SUITE_P(AllPositions, MoveGenerationTest,
    ::testing::ValuesIn(MoveGenerationTestCases));

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

void parse_test_cases_from_file(std::string cases_file) {
    toml::parse_result result = toml::parse_file(cases_file);

    if (!result) {
        std::println(stderr, "ERROR:: Could not parse file {}, {}", cases_file,
            result.error().description());
    }

    toml::table tbl = std::move(result.table());

    for (const auto& [category, cases] : *tbl["generationcount"].as_table()) {

        for (const auto& item : *cases.as_array()) {
            const auto& tcase = *item.as_table();

            core::test::MoveCounts move_count{
                .pawns = tcase["pawns"].value_or<uint16_t>(0),
                .knights = tcase["knights"].value_or<uint16_t>(0),
                .bishops = tcase["bishops"].value_or<uint16_t>(0),
                .rooks = tcase["rooks"].value_or<uint16_t>(0),
                .queens = tcase["queens"].value_or<uint16_t>(0),
                .kings = tcase["kings"].value_or<uint16_t>(0),
            };

            auto fen = tcase["fen"].value<std::string>();

            if (!fen) {
                std::println(stderr,
                    "ERROR:: Could not parse test case, FEN is missing");
                continue;
            }

            core::test::MoveGenerationTestCases.emplace_back(
                fen.value(), move_count);
        }
    }
}

}  // namespace core::test

int main(int argc, char** argv) {
    core::test::parse_test_cases_from_file(
        (std::filesystem::canonical(argv[0]).parent_path() / "cases.toml")
            .string());

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
