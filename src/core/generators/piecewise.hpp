#include "core/types.hpp"
#include <core/generators.hpp>

namespace core::generators::pawns {
    void gen_pawns_moves(MoveGenerator& generator);
    void gen_check_blocks(MoveGenerator& generator, bitboard allowed);

    bitboard get_advances(bitboard pawns, bitboard blockers, Color color);
    bitboard east_attacks(bitboard pawns, Color color);
    bitboard west_attacks(bitboard pawns, Color color);

} // namespace core::generators::pawns

namespace core::generators::sliders {
    void gen_slider_moves(MoveGenerator& generator);
    void gen_check_blocks(MoveGenerator& generator, bitboard allowed);
} // namespace core::generators::sliders

namespace core::generators::knights {
    extern bitboard available_moves[64];

    void initialize_table();
    void gen_knights_moves(MoveGenerator& generator);
    void gen_check_blocks(MoveGenerator& generator, bitboard allowed);
} // namespace core::generators::knights

namespace core::generators::kings {
    extern bitboard available_moves[64];

    void initialize_table();

    bitboard pin_rook_xrays(const Board& board);
    bitboard pin_bishop_xrays(const Board& board);

    bool is_enpassant_pinned(MoveGenerator& board);

    template <bool generate_castle = true>
    extern void gen_king_moves(MoveGenerator& generator);

    bitboard checking_pieces(const Board& board);
} // namespace core::generators::kings
