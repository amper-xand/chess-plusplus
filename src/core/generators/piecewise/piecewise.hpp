#include "../generators.hpp"

namespace Game::Generators::Pawns {
    MoveGenerator& gen_pawns_moves(MoveGenerator& generator);
    MoveGenerator& gen_check_blocks(MoveGenerator& generator, bitboard allowed);

    bitboard get_advances(bitboard pawns, bitboard blockers, bool color);
    bitboard get_advances(bitboard pawns, bitboard blockers,
                          Colors::Color color);

    bitboard east_attacks(bitboard pawns, bool color);
    bitboard east_attacks(bitboard pawns, Colors::Color color);

    bitboard west_attacks(bitboard pawns, bool color);
    bitboard west_attacks(bitboard pawns, Colors::Color color);

} // namespace Game::Generators::Pawns

namespace Game::Generators::Sliders {
    MoveGenerator& gen_slider_moves(MoveGenerator& generator);
    MoveGenerator& gen_check_blocks(MoveGenerator& generator, bitboard allowed);
} // namespace Game::Generators::Sliders

namespace Game::Generators::Knights {
    extern bitboard available_moves[64];

    void initialize_table();
    MoveGenerator& gen_knights_moves(MoveGenerator& generator);
    MoveGenerator& gen_check_blocks(MoveGenerator& generator, bitboard allowed);
} // namespace Game::Generators::Knights

namespace Game::Generators::Kings {
    extern bitboard available_moves[64];

    void initialize_table();

    bitboard pin_rook_xrays(Board& board);
    bitboard pin_bishop_xrays(Board& board);

    bool is_enpassant_pinned(MoveGenerator& board);

    template <bool generate_castle = true>
    extern MoveGenerator& gen_king_moves(MoveGenerator& generator);

    bitboard checking_pieces(Board& board);
} // namespace Game::Generators::Kings
