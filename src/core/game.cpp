#include "game.hpp"

#include "../utils/utils.hpp"

namespace Game::Pieces {
    // clang-format off
    Piece char_to_piece(char c) {
        switch (c) {
        case 'p': case 'P':
            return PAWNS;

        case 'n': case 'N':
            return KNIGHTS;

        case 'b': case 'B':
            return BISHOPS;

        case 'r': case 'R':
            return ROOKS;

        case 'q': case 'Q':
            return QUEENS;

        case 'k': case 'K':
            return KINGS;
        }

        return PAWNS;
    }
    // clang-format on

    char piece_to_char(Piece piece, Colors::Color color) {
        switch (piece) {
        case PAWNS:
            return color ? 'P' : 'p';
        case KNIGHTS:
            return color ? 'N' : 'n';
        case BISHOPS:
            return color ? 'B' : 'b';
        case ROOKS:
            return color ? 'R' : 'r';
        case QUEENS:
            return color ? 'Q' : 'q';
        case KINGS:
            return color ? 'K' : 'k';
        case NONE:
            break;
        }

        return 'X';
    }
} // namespace Game::Pieces

namespace Game {
    Board Board::parse_fen_string(std::string fen) {
        Board board;

        for (auto piece : Pieces::AllPieces)
            board.pieces[piece] = 0ul;

        board.white = 0ul;
        board.black = 0ul;

        square index = 63;

        for (char c : fen) {
            if (index > 64) {
                break;
            }

            if (std::isdigit(c)) {
                index -= c - '0'; // Skip empty squares.
            } else if (c == '/') {
                continue; // Move to the next rank.
            } else {
                Colors::Color color =
                    std::isupper(c) ? Colors::WHITE : Colors::BLACK;
                Pieces::Piece piece = Pieces::char_to_piece(c);

                board.colors[color] |= Utils::bit_at(index);
                board.pieces[piece] |= Utils::bit_at(index);
                index--;
            }
        }

        board.turn = Colors::WHITE;

        return board;
    }

    void Board::print_board() {
        std::printf("\n");
        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 7; file >= 0; --file) {
                int index = rank * 8 + file;
                if (is_square_occupied(index))
                    std::printf(" %c ", Pieces::piece_to_char(piece_at(index),
                                                              color_at(index)));
                else
                    std::printf(" - ");

                if (index % 8 == 0)
                    std::printf("\n");
            }
        }
        std::printf("\n");
    }

    bool Board::is_square_occupied(square index) {
        return Utils::is_set_at(index, white | black);
    }

    Game::Colors::Color Board::color_at(square index) {
        if (Utils::is_set_at(index, white))
            return Colors::WHITE;
        return Colors::BLACK;
    }

    Game::Pieces::Piece Board::piece_at(square index) {
        for (auto piece : Pieces::AllPieces) {
            if (Utils::is_set_at(index, pieces[piece]))
                return piece;
        }

        return Pieces::NONE;
    }

    void Board::make_move(Move move) {
        colors[turn] &= ~Utils::bit_at(move.from);
        colors[turn] |= Utils::bit_at(move.to);

        pieces[move.piece_moved] &= ~Utils::bit_at(move.from);
        pieces[move.piece_moved] |= Utils::bit_at(move.to);

        turn = Colors::BothColors[!turn];
    }

} // namespace Game
