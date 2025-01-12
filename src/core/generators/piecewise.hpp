#include <core/generators.hpp>

namespace core::generators::pawns {
    MoveGenerator& gen_pawns_moves(MoveGenerator& generator);
    MoveGenerator& gen_check_blocks(MoveGenerator& generator, bitboard allowed);

    bitboard get_advances(bitboard pawns, bitboard blockers, Color color);
    bitboard east_attacks(bitboard pawns, Color color);
    bitboard west_attacks(bitboard pawns, Color color);

} // namespace core::generators::pawns

namespace core::generators::sliders {
    MoveGenerator& gen_slider_moves(MoveGenerator& generator);
    MoveGenerator& gen_check_blocks(MoveGenerator& generator, bitboard allowed);
} // namespace core::generators::sliders

namespace core::generators::knights {
    extern bitboard available_moves[64];

    void initialize_table();
    MoveGenerator& gen_knights_moves(MoveGenerator& generator);
    MoveGenerator& gen_check_blocks(MoveGenerator& generator, bitboard allowed);
} // namespace core::generators::knights

namespace core::generators::kings {
    extern bitboard available_moves[64];

    void initialize_table();

    bitboard pin_rook_xrays(Board& board);
    bitboard pin_bishop_xrays(Board& board);

    bool is_enpassant_pinned(MoveGenerator& board);

    template <bool generate_castle = true>
    extern MoveGenerator& gen_king_moves(MoveGenerator& generator);

    bitboard checking_pieces(Board& board);
} // namespace core::generators::kings
