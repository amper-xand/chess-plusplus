#include <core/types.hpp>

namespace core::generators::magic {

    namespace rooks {
        bitboard get_avail_moves(bitboard blockers, square index);
        bitboard get_slider(square index);
    } // namespace Rooks

    namespace bishops {
        bitboard get_avail_moves(bitboard blockers, square index);
        bitboard get_slider(square index);
    } // namespace Bishops

    void initialize_magic_tables();

} // namespace core::generators::Magic
