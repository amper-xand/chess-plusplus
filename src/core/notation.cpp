#include "notation.hpp"

#include <cctype>
#include <exception>
#include <format>
#include <ranges>
#include <string>
#include <string_view>

std::tuple<core::Piece, core::Color> core::notation::cto_piece(char c) {
    Color color = !!std::isupper(c);

    // clang-format off
    switch (c) {
    case 'p': case 'P':
        return {Piece::PAWNS, color};

    case 'n': case 'N':
        return {Piece::KNIGHTS, color};

    case 'b': case 'B':
        return {Piece::BISHOPS, color};

    case 'r': case 'R':
        return {Piece::ROOKS, color};

    case 'q': case 'Q':
        return {Piece::QUEENS, color};

    case 'k': case 'K':
        return {Piece::KINGS, color};
    }
    // clang-format on

    throw notation::invalid_token(std::format(  //
        "ERROR:: Unexpected character {} when parsing core::Piece", c));
}

core::square core::notation::strto_square(std::string_view str) {
    if (str.length() != 2) {
        throw notation::malformed_data(std::format(  //
            "ERROR:: data='{}' is not of length=2 while parsing core::square",
            str));
    }

    if ((std::string("abcdefgh").find(str[0]) == std::string::npos) ||
        (std::string("12345678").find(str[1]) == std::string::npos)) {
        throw notation::invalid_token(std::format(  //
            "ERROR:: Unexpected token in '{}' while parsing core::square",
            str));
    }

    int rank = str[0] - 'a';
    int file = str[1] - '1';

    return square::at(rank, file);
}

core::Color core::notation::FEN::parse_active_color(char c) {
    if (c == 'w') {
        return Color::WHITE;
    }

    if (c == 'b') {
        return Color::BLACK;
    }

    throw notation::invalid_token(std::format(  //
        "ERROR:: Unexpected character {} while parsing core::Color", c));
}

core::Board::State::Castling core::notation::FEN::parse_castling_availability(
    std::string_view str) {
    if (str.find_first_not_of("KQkq-") != std::string::npos) {
        throw notation::invalid_token(std::format(  //
            "ERROR:: Unexpected token in '{}'"
            "while parsing core::Board::State::Castling",
            str));
    }

    Board::State::Castling castling = {false, false, false, false};

    if (str == "-") {
        return castling;
    }

    if (str.find('K') != std::string::npos) {
        castling.white_right = true;
    }

    if (str.find('k') != std::string::npos) {
        castling.black_right = true;
    }

    if (str.find('Q') != std::string::npos) {
        castling.white_left = true;
    }

    if (str.find('q') != std::string::npos) {
        castling.black_right = true;
    }

    return castling;
}

core::square core::notation::FEN::parse_en_passant_target_square(
    std::string_view str) {
    if (str == "-") {
        return 65;
    }

    return strto_square(str);
}

const core::notation::FEN core::notation::FEN::parse_string(
    std::string_view fen) {
    FEN parsed = {.active_color = Color::WHITE,
        .castling_availability = {true, true, true, true},
        .en_passant_target_square = 65,
        .half_move_clock = 0,
        .full_move_number = 0};

    auto fields = std::views::split(fen, ' ');

    for (auto const [index, field] : std::views::enumerate(fields)) {
        auto data = std::string_view(field.begin(), field.end());

        try {
            parsed.parse_field(index, data);
        } catch (...) {
            std::throw_with_nested(notation::parse_error(std::format(  //
                "ERROR:: Failed to parse core::notation::FEN '{}'", fen)));
        }
    }

    return parsed;
}

void core::notation::FEN::parse_field(int index, std::string_view data) {
    constexpr int PLACEMENT_DATA = 0;
    constexpr int ACTIVE_COLOR = 1;
    constexpr int CASTLING_AVAILABILITY = 2;
    constexpr int EN_PASSANT_TARGET_SQUARE = 3;
    constexpr int HALF_MOVE_CLOCK = 4;
    constexpr int FULL_MOVE_NUMBER = 5;
    constexpr int INVALID_FIELD = 6;

    if (index == PLACEMENT_DATA) {
        if (data.find_first_not_of("PNBRQKpnbrqk12345678/") !=
            std::string_view::npos) {
            throw notation::invalid_token(std::format(  //
                "ERROR:: Unexpected token in '{}' while parsing PLACEMENT_DATA",
                data));
        }

        this->placement_data = std::string(data);

        return;
    }

    auto parse_or_throw =  //
        [](auto&& parser, auto&& value, const char* field_name) {
            try {
                return parser(value);
            } catch (...) {
                std::throw_with_nested(notation::parse_error(std::format(
                    "ERROR:: Failed parsing {} '{}'", field_name, value)));
            }
        };

    if (index == ACTIVE_COLOR) {
        this->active_color = parse_or_throw(  //
            FEN::parse_active_color, data[0], "ACTIVE_COLOR");
        return;
    }

    if (index == CASTLING_AVAILABILITY) {
        this->castling_availability = parse_or_throw(  //
            FEN::parse_castling_availability, data, "CASTLING_AVAILABILITY");
        return;
    }

    if (index == EN_PASSANT_TARGET_SQUARE) {
        this->en_passant_target_square = parse_or_throw(  //
            FEN::parse_en_passant_target_square, data,
            "EN_PASSANT_TARGET_SQUARE");
        return;
    }

    if (index == HALF_MOVE_CLOCK) {
        this->half_move_clock = parse_or_throw(  //
            [&](std::string_view data) { return std::stoi((std::string)data); },
            data, "HALF_MOVE_CLOCK");
        return;
    }

    if (index == FULL_MOVE_NUMBER) {
        this->half_move_clock = parse_or_throw(  //
            [&](std::string_view data) { return std::stoi((std::string)data); },
            data, "FULL_MOVE_NUMBER");
        return;
    }

    if (index >= INVALID_FIELD) {
        throw notation::malformed_data(
            std::format("ERROR:: Unexpected field '{}'", data));
    }
}

core::Board core::notation::FEN::get_board() {
    namespace views = std::views;

    Board board;

    auto ranks = views::split(this->placement_data, '/');

    if (std::ranges::distance(ranks) != 8) {
        throw notation::malformed_data(std::format(  //
            "ERROR:: Expected field of length `8` got "
            "data='{}' while creating Board",
            ranks));
    }

    int rank = 7;

    for (auto data : ranks) {
        int file = 7;

        for (char d : data) {
            if (std::isdigit(d)) {
                file -= d - '0';
            } else {
                auto [piece, color] = cto_piece(d);

                square index = square::at(rank, file);

                board.pieces[piece] |= index.bb();
                board.colors[color] |= index.bb();

                file--;
            }
        }

        rank--;
    }

    board.turn = this->active_color;
    board.castling = this->castling_availability;
    board.en_passant = this->en_passant_target_square;
    board.fifty_move_rule_counter = this->half_move_clock;

    return board;
}
