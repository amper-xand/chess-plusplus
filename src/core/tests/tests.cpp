#include <core/generation.hpp>
#include <core/representation.hpp>

#include <cstdint>
#include <format>
#include <print>
#include <string>

namespace core::test {

template <typename Expected>
class Test {
   protected:
    Expected expected;

    Test(uint8_t expected) : expected(expected) {}

    struct Result {
        std::string title, msg;
        bool passed;
        Expected expected, produced;

        std::string message() {
            constexpr int width = 40;

            // Format title line with centered title
            std::string base = std::format(
                "{:=^{}}\n", std::format("==[ {} ]==", title), width);

            std::string status = std::format(
                "{}\n", passed ? "✔ Test Passed" : "✗ Test Failed");

            std::string user_msg =
                msg.empty() ? "" : std::format("Info: {}\n", msg);

            std::string details = std::format("Expected:\n{}\nProduced:\n{}\n",
                                              expected, produced);

            std::string end =
                std::format("{:=^{}}\n", "", width);  // Bottom bar same width

            return base + status + user_msg + details + end;
        }
    };

   public:
    virtual Result run() = 0;
};

class PawnGenTest : public Test<uint8_t>, private core::MoveGenerator {
   public:
    PawnGenTest(const core::Board& board, uint8_t expected_moves)
        : core::MoveGenerator(board), Test(expected_moves) {}

    Result run() override {
        generate_pawn_moves();

        uint8_t produced = get_generated_moves().size();
        bool passed = produced == expected;

        return Result{.title = "Pawn generation logic test",
                      .passed = passed,
                      .expected = expected,
                      .produced = produced};
    }
};

}  // namespace core::test

int main() {
    auto position1 = core::Board::parse_fen_repr(
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::print("{}", core::test::PawnGenTest(position1, 16).run().message());
}
