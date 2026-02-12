#include <core/generation.hpp>
#include <core/notation.hpp>
#include <core/types.hpp>

#include <exception>
#include <iostream>
#include <iterator>
#include <print>
#include <random>
#include <string>

void play_game(const std::string& fen_str);

void show_nested_exception(const std::exception& e, int level = 0) {
    std::println(std::cerr, "{}{}", std::string(level * 2, ' '), e.what());

    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& nested) {
        show_nested_exception(nested, level + 1);
    }
}

int main() {
    while (true) {
        std::println("Start a new game");
        std::println(
            "(s) to start the a new game,"
            "(f) to enter a game FEN,"
            "(q) to quit");

        std::string option;

        if (!std::getline(std::cin, option)) {
            if (std::cin.eof()) {
                return 0;  // End of input: exit gracefully
            }

            if (std::cin.fail()) {
                std::cin.clear();
                std::cin.ignore(
                    std::numeric_limits<std::streamsize>::max(), '\n');

                std::println(std::cerr, "Error:: Input error");

                continue;
            }
        }

        if (option == "s") {
            play_game(
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }

        if (option == "f") {
            std::string fen;

            if (!std::getline(std::cin, fen)) {
                if (std::cin.eof()) {
                    return 0;  // End of input: exit gracefully
                }

                if (std::cin.fail()) {
                    std::cin.clear();
                    std::cin.ignore(
                        std::numeric_limits<std::streamsize>::max(), '\n');

                    std::println(std::cerr, "Error:: Input error");

                    continue;
                }
            }

            play_game(fen);
        }

        if (option == "q") {
            return 0;
        }
    }
}

void play_game(const std::string& fen_str) {
    core::Board board;

    try {
        board = core::notation::FEN::parse_string(fen_str);

    } catch (const core::notation::parse_error& err) {
        show_nested_exception(err);
        return;
    }

    std::mt19937 gen(42);  // fixed seed
    while (true) {
        auto moves = core::generation::generate_moves(board);

        if (moves.empty()) {
            throw std::runtime_error("No moves available");
        }

        std::vector<core::Move> example;
        example.reserve(1);

        std::sample(moves.begin(), moves.end(), std::back_inserter(example), 1,
            std::mt19937{std::random_device{}()});

        core::notation::MoveLAN exampleLAN =
            core::notation::MoveLAN::from_move(example[0]);

        std::println("{}", core::notation::draw_board_ascii(board));
        std::println("Select a move using long algebraic notation.");
        std::println("Example: {}", exampleLAN.to_string());

        std::string selected_str;

        if (!std::getline(std::cin, selected_str)) {
        }

        auto selected = core::notation::MoveLAN::parse_string(selected_str);

        bool selected_move = false;

        for (auto m : moves) {
            if ((selected_move = selected.matches_move(m))) {
                board.play(m);
                // std::println("INFO:: Played move {} {} {} {}",
                //     (core::square_t)m.from, (core::square_t)m.to,
                //     core::notation::piece_toname(m.moved),
                //     core::notation::piece_toname(m.target));
                break;
            }
        }

        // if (!selected_move) {
        //     std::println("WARNING:: Failed to select move {} {}",
        //         (core::square_t)selected.from, (core::square_t)selected.to);
        //
        //     std::println("WARNING:: EXAMPLE {} {} {}",
        //         (core::square_t)example[0].from, (core::square_t)example[0].to,
        //         (core::piece_t)example[0].promotion);
        // }
    }
}
