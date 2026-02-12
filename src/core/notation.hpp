#include "types.hpp"

#include <stdexcept>
#include <string>

namespace core::notation {

struct parse_error : std::runtime_error {
    explicit parse_error(const std::string& msg) : std::runtime_error(msg) {}
};
struct invalid_token : parse_error {
    explicit invalid_token(const std::string& msg) : parse_error(msg) {}
};
struct malformed_data : parse_error {
    explicit malformed_data(const std::string& msg) : parse_error(msg) {}
};

square strto_square(std::string_view str) noexcept(false);
std::string square_tostr(square square) noexcept(false);

std::tuple<Piece, Color> cto_piece(char c) noexcept(false);
char piece_toc(Piece piece, Color color) noexcept(false);
std::string piece_toname(Piece piece) noexcept(false);

const std::string draw_board_ascii(const Board& board) noexcept(false);

// Represents the fields of a move in Long Algebraic Notation (as in UCI)
struct MoveLAN {
    square from;
    square to;
    Piece promotion;

    static const MoveLAN parse_string(std::string_view lan) noexcept(false);
    static const MoveLAN from_move(Move move) noexcept(true);

    bool matches_move(Move move) noexcept(true);

    std::string to_string();
};

}  // namespace core::notation

// Forsyth–Edwards Notation
namespace core::notation::FEN {

Board parse_string(std::string_view fen) noexcept(false);

void parse_fieldstr_into(
    Board& target, int index, std::string_view data) noexcept(false);

Board::Positions parse_placement_data(  //
    std::string_view data) noexcept(false);

Color parse_active_color(char c) noexcept(false);

Board::State::Castling parse_castling_availability(  //
    std::string_view str) noexcept(false);

square parse_en_passant_target_square(  //
    std::string_view str) noexcept(false);
}  // namespace core::notation::FEN
