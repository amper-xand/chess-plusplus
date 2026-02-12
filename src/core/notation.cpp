#include "notation.hpp"

#include <cctype>
#include <format>

#include <exception>

#include <functional>
#include <ranges>

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

    int file = 'h' - str[0];
    int rank = str[1] - '1';

    return square::at(rank, file);
}

std::string core::notation::square_tostr(square square) {
    char file = std::string("hgfedcba").at(square.column());
    char rank = std::string("12345678").at(square.row());

    return (std::string){file, rank};
}

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

char core::notation::piece_toc(Piece piece, Color color) {
    // clang-format off
    switch (piece) {
        case Piece::PAWNS   : return color ? 'P' : 'p';
        case Piece::KNIGHTS : return color ? 'N' : 'n';
        case Piece::BISHOPS : return color ? 'B' : 'b';
        case Piece::ROOKS   : return color ? 'R' : 'r';
        case Piece::QUEENS  : return color ? 'Q' : 'q';
        case Piece::KINGS   : return color ? 'K' : 'k';

        case Piece::NONE: return '-';
    }

    throw notation::invalid_token(std::format( //
                "ERROR:: Could not convert core::Piece {} into char", (piece_t) piece));
    // clang-format on
}

std::string core::notation::piece_toname(Piece piece) {
    if (piece == Piece::PAWNS) return "pawn";
    if (piece == Piece::KNIGHTS) return "knight";
    if (piece == Piece::BISHOPS) return "bishop";
    if (piece == Piece::ROOKS) return "rook";
    if (piece == Piece::QUEENS) return "queen";
    if (piece == Piece::KINGS) return "king";

    if (piece == Piece::NONE) return "none";

    throw notation::invalid_token(std::format(  //
        "ERROR:: Could not map core::Piece {} into name", (piece_t)piece));
}

const std::string core::notation::draw_board_ascii(const Board& board) {
    using namespace std::placeholders;

    std::string str;

    for (int rank : std::views::iota(0, 8) | std::views::reverse) {
        auto files = std::views::iota(0, 8) | std::views::reverse;
        auto indices =
            files | std::views::transform(std::bind(square::at, rank, _1));

        auto pieces = indices | std::views::transform([&](square index) {
            return board.piece(index);
        });

        auto colors = indices | std::views::transform([&](square index) {
            return board.color(index);
        });

        auto tiles_chars =
            std::views::zip_transform(notation::piece_toc, pieces, colors);

        for (auto tile : tiles_chars) {
            str += ' ';
            str += tile;
            str += ' ';
        }

        str += '\n';
    }

    return str;
}

// Takes a string formated with Forsyth–Edwards Notation
// then it parses every field.
// If the last fields aren't included then it fills them with defaults.
core::Board core::notation::FEN::parse_string(std::string_view fen) {
    Board parsed;

    parsed.active_color = Color::WHITE;
    parsed.castling_availability = {true, true, true, true};
    parsed.en_passant_target_square = square::out_of_bounds;
    parsed.halfmove_clock = 0;
    parsed.fullmove_number = 1;

    auto fields = std::views::split(fen, ' ');

    for (auto const [index, field] : std::views::enumerate(fields)) {
        auto data = std::string_view(field.begin(), field.end());

        try {
            FEN::parse_fieldstr_into(parsed, index, data);
        } catch (...) {
            std::throw_with_nested(notation::parse_error(std::format(  //
                "ERROR:: Failed to parse core::Board '{}'", fen)));
        }
    }

    return parsed;
}

// Parses the data of a field and parses it base on its position.
//
// Helper function to fill data during parsing.
void core::notation::FEN::parse_fieldstr_into(
    Board& target, int index, std::string_view data) {
    constexpr int PLACEMENT_DATA = 0;
    constexpr int ACTIVE_COLOR = 1;
    constexpr int CASTLING_AVAILABILITY = 2;
    constexpr int EN_PASSANT_TARGET_SQUARE = 3;
    constexpr int HALF_MOVE_CLOCK = 4;
    constexpr int FULL_MOVE_NUMBER = 5;
    constexpr int INVALID_FIELD = 6;

    auto parse_or_throw =  //
        [](auto&& parser, auto&& value, const char* field_name) {
            try {
                return parser(value);
            } catch (...) {
                std::throw_with_nested(notation::parse_error(std::format(
                    "ERROR:: Failed parsing {} '{}'", field_name, value)));
            }
        };

    if (index == PLACEMENT_DATA) {
        static_cast<Positions&>(target) = parse_or_throw(  //
            FEN::parse_placement_data, data, "PLACEMENT_DATA");
        return;
    }

    if (index == ACTIVE_COLOR) {
        target.active_color = parse_or_throw(  //
            FEN::parse_active_color, data[0], "ACTIVE_COLOR");
        return;
    }

    if (index == CASTLING_AVAILABILITY) {
        target.castling_availability = parse_or_throw(  //
            FEN::parse_castling_availability, data, "CASTLING_AVAILABILITY");
        return;
    }

    if (index == EN_PASSANT_TARGET_SQUARE) {
        target.en_passant_target_square = parse_or_throw(  //
            FEN::parse_en_passant_target_square, data,
            "EN_PASSANT_TARGET_SQUARE");
        return;
    }

    if (index == HALF_MOVE_CLOCK) {
        target.halfmove_clock = parse_or_throw(  //
            [&](std::string_view data) { return std::stoi((std::string)data); },
            data, "HALF_MOVE_CLOCK");
        return;
    }

    if (index == FULL_MOVE_NUMBER) {
        target.fullmove_number = parse_or_throw(  //
            [&](std::string_view data) { return std::stoi((std::string)data); },
            data, "FULL_MOVE_NUMBER");
        return;
    }

    if (index >= INVALID_FIELD) {
        throw notation::malformed_data(
            std::format("ERROR:: Unexpected field '{}'", data));
    }
}

core::Board::Positions core::notation::FEN::parse_placement_data(
    std::string_view data) {
    if (data.find_first_not_of("PNBRQKpnbrqk12345678/") !=
        std::string_view::npos) {
        throw notation::invalid_token(std::format(  //
            "ERROR:: Unexpected token in '{}'"
            "while parsing core::Board::Postions",
            data));
    }

    namespace views = std::views;

    Board::Positions parsed;

    auto ranks = views::split(data, '/');

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

                parsed.pieces[piece] |= index.bb();
                parsed.colors[color] |= index.bb();

                file--;
            }
        }

        rank--;
    }

    return parsed;
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
        return square::out_of_bounds;
    }

    return strto_square(str);
}

const core::notation::MoveLAN core::notation::MoveLAN::parse_string(
    std::string_view lan) {
    MoveLAN parsed{65, 65, Piece::NONE};

    parsed.from = notation::strto_square(lan.substr(0, 2));
    parsed.to = notation::strto_square(lan.substr(2, 2));

    if (lan.length() == 5) {
        auto [piece, _] = notation::cto_piece(lan[4]);
        parsed.promotion = piece;
    }

    return parsed;
}

const core::notation::MoveLAN core::notation::MoveLAN::from_move(
    Move move) noexcept(true) {
    return MoveLAN{move.from, move.to, move.promotion};
}

bool core::notation::MoveLAN::matches_move(Move move) noexcept(true) {
    if (this->from != move.from) return false;
    if (this->to != move.to) return false;
    if (this->promotion != move.promotion) return false;

    return true;
}

std::string core::notation::MoveLAN::to_string() {
    auto str = square_tostr(this->from) + square_tostr(this->to);

    if (!this->promotion.isNone()) {
        str += piece_toc(this->promotion, false);
    }

    return str;
}
