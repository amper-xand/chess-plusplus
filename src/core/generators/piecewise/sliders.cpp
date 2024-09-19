#include "piecewise.hpp"

#include "../magic/magic.hpp"
#include "../helpers.hpp"

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

}
