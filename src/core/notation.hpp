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

std::tuple<Piece, Color> cto_piece(char c) noexcept(false);
square strto_square(std::string_view str) noexcept(false);

char piece_toc(Piece piece, Color color) noexcept(false);
std::string piece_toname(Piece piece) noexcept(false);

const std::string draw_board_ascii(const Board& board) noexcept(false);

// Represents the fields of the Forsyth–Edwards Notation
struct FEN {
    std::string placement_data;
    Color active_color;
    Board::State::Castling castling_availability;
    square en_passant_target_square;
    int half_move_clock;
    int full_move_number;

    static const FEN parse_string(std::string_view fen) noexcept(false);

    Board get_board() const noexcept(false);

   protected:
    void parse_field(int index, std::string_view data) noexcept(false);

    static Color parse_active_color(char c) noexcept(false);

    static Board::State::Castling parse_castling_availability(  //
        std::string_view str) noexcept(false);

    static square parse_en_passant_target_square(  //
        std::string_view str) noexcept(false);
};

}  // namespace core::notation
