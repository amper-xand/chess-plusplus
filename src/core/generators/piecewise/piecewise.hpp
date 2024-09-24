#include "../generators.hpp"

namespace Game::Generators::Pawns {
    MoveGenerator& gen_pawns_moves(MoveGenerator& generator);

    bitboard get_advances(bitboard pawns, bitboard blockers, bool color);
    bitboard get_advances(bitboard pawns, bitboard blockers,
                          Colors::Color color);

    bitboard east_attacks(bitboard pawns, bool color);
    bitboard east_attacks(bitboard pawns, Colors::Color color);

    bitboard west_attacks(bitboard pawns, bool color);
    bitboard west_attacks(bitboard pawns, Colors::Color color);

} // namespace Game::Generators::Pawns

namespace Game::Generators::Sliders {
    MoveGenerator& gen_rooks_moves(MoveGenerator& generator);

    MoveGenerator& gen_bishops_moves(MoveGenerator& generator);

    MoveGenerator& gen_queens_moves(MoveGenerator& generator);

} // namespace Game::Generators::Sliders

namespace Game::Generators::Knights {
    extern bitboard available_moves[64];

    void initialize_table();
    MoveGenerator& gen_knights_moves(MoveGenerator& generator);

} // namespace Game::Generators::Knights

namespace Game::Generators::Kings {
    extern bitboard available_moves[64];

    void initialize_table();

    bitboard get_pinned_pieces(Board board);

    bool is_enpassant_pinned(Board board);

    MoveGenerator& gen_king_moves(MoveGenerator& generator);

} // namespace Game::Generators::Kings
