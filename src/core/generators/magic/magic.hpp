#include "../../representation/representation.hpp"

namespace Game::Generators::Magic {

    namespace Rooks {
        bitboard get_avail_moves(bitboard blockers, square index);
    }

    namespace Bishops {
        bitboard get_avail_moves(bitboard blockers, square index);
    }

    void initialize_magic_tables();

} // namespace Game::Generators::Magic
