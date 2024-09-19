#include "piecewise.hpp"

#include "../../../utils/utils.hpp"
#include "../helpers.hpp"

namespace Game::Generators::Knights {
    bitboard available_moves[64];

    bitboard gen_knight_moves(square index) {
        bitboard moves = 0;
        square col = Utils::column(index);
        square row = Utils::row(index);

        // Top-most left
        if (row <= 5 && 1 <= col)
            moves |= Utils::bit_at(Utils::index_at(row + 2, col - 1));

        // Bottom-most left
        if (2 <= row && 1 <= col)
            moves |= Utils::bit_at(Utils::index_at(row - 2, col - 1));

        // Top-most right
        if (row <= 5 && col <= 6)
            moves |= Utils::bit_at(Utils::index_at(row + 2, col + 1));

        // Bottom-most right
        if (2 <= row && col <= 6)
            moves |= Utils::bit_at(Utils::index_at(row - 2, col + 1));

        // Top left-most
        if (row <= 6 && 2 <= col)
            moves |= Utils::bit_at(Utils::index_at(row + 1, col - 2));

        // Bottom left-most
        if (1 <= row && 2 <= col)
            moves |= Utils::bit_at(Utils::index_at(row - 1, col - 2));

        // Top right-most
        if (row <= 6 && col <= 5)
            moves |= Utils::bit_at(Utils::index_at(row + 1, col + 2));

        // Bottom right-most
        if (1 <= row && col <= 5)
            moves |= Utils::bit_at(Utils::index_at(row - 1, col + 2));

        return moves;
    }

    void initialize_table() {
        for (square index = 0; index < 64; ++index) {
            available_moves[index] = gen_knight_moves(index);
        }
    }

    MoveGenerator& gen_knights_moves(MoveGenerator& generator) {
        auto knight_generator = [](bitboard blockers, square index) {
            return Knights::available_moves[index];
        };

        return Helpers::moves_from_generator<knight_generator, Pieces::KNIGHTS>(
            generator);
    }

} // namespace Game::Generators::Knights
