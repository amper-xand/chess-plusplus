#pragma once

#include <core/types.hpp>

namespace core::generation::magic::rooks {
bitboard get_avail_moves(bitboard blockers, square index);
bitboard get_slider(square index);
}  // namespace core::generation::magic::rooks

namespace core::generation::magic::bishops {
bitboard get_avail_moves(bitboard blockers, square index);
bitboard get_slider(square index);
}  // namespace core::generation::magic::bishops
