#include "piecewise.hpp"

#include "../helpers.hpp"
#include "../magic/magic.hpp"

namespace Game::Generators::Sliders {
    MoveGenerator& gen_rooks_moves(MoveGenerator& generator) {
        return Helpers::moves_from_generator<Magic::Rooks::get_avail_moves,
                                             Pieces::ROOKS>(generator);
    }

    MoveGenerator& gen_bishops_moves(MoveGenerator& generator) {
        return Helpers::moves_from_generator<Magic::Bishops::get_avail_moves,
                                             Pieces::BISHOPS>(generator);
    }

    MoveGenerator& gen_queens_moves(MoveGenerator& generator) {
        auto queen_generator = [](bitboard blockers, square index) {
            return Magic::Rooks::get_avail_moves(blockers, index) |
                   Magic::Bishops::get_avail_moves(blockers, index);
        };

        return Helpers::moves_from_generator<queen_generator, Pieces::QUEENS>(
            generator);
    }

    MoveGenerator& gen_check_blocks(MoveGenerator& generator,
                                    bitboard allowed) {

        auto& board = generator.board;

        bitboard blockers = board.all();
        bitboard capturable = board.enemies();

        // Pinned pieces cannot move or capture a piece giving check
        bitboard pinned = generator.pins.absolute | generator.pins.partial;

        bitboard::scan(board.allied(Pieces::BISHOPS).pop(pinned), [&](square index) {
            bitboard moves =
                Magic::Bishops::get_avail_moves(blockers, index).mask(allowed);
            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Pieces::BISHOPS, index, moves, captures);
        });

        bitboard::scan(board.allied(Pieces::ROOKS).pop(pinned), [&](square index) {
            bitboard moves =
                Magic::Rooks::get_avail_moves(blockers, index).mask(allowed);
            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Pieces::ROOKS, index, moves, captures);
        });

        bitboard::scan(board.allied(Pieces::QUEENS).pop(pinned), [&](square index) {
            bitboard moves =
                Magic::Rooks::get_avail_moves(blockers, index)
                    .join(Magic::Bishops::get_avail_moves(blockers, index))
                    .mask(allowed);

            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Pieces::QUEENS, index, moves, captures);
        });

        return generator;
    }
} // namespace Game::Generators::Sliders
