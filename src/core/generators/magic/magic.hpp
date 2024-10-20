#include "../../representation/representation.hpp"

namespace Game::Generators::Magic {

    namespace Rooks {
        bitboard get_avail_moves(bitboard blockers, square index);
        bitboard get_slider(square index);
    } // namespace Rooks

    namespace Bishops {
        bitboard get_avail_moves(bitboard blockers, square index);
        bitboard get_slider(square index);
    } // namespace Bishops

    void initialize_magic_tables();

} // namespace Game::Generators::Magic
