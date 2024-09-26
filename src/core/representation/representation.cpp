#include "representation.hpp"

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

    char piece_to_char(Piece piece, Colors::Color color) {
        switch (piece) {

        case PAWNS   : return color ? 'P' : 'p';
        case KNIGHTS : return color ? 'N' : 'n';
        case BISHOPS : return color ? 'B' : 'b';
        case ROOKS   : return color ? 'R' : 'r';
        case QUEENS  : return color ? 'Q' : 'q';
        case KINGS   : return color ? 'K' : 'k';

        case NONE: break;
        }

        return 'X';
    }
    // clang-format on
} // namespace Game::Pieces
