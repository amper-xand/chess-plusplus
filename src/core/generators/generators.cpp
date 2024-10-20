#include "generators.hpp"

#include "piecewise/piecewise.hpp"

#include "../representation/representation.hpp"
#include "magic/magic.hpp"

#include <climits>
#include <cstdio>
#include <span>
#include <stdexcept>
#include <strings.h>
#include <vector>

namespace Game::Generators {
    MoveGenerator::MoveGenerator(Board board) { this->board = board; }

    std::vector<Move> MoveGenerator::get_generated() {
        if (next_move == moves.begin())
            // No moves have been generated
            return std::vector<Move>(0);

        return std::vector(moves.begin(), next_move);
    }

    Move& MoveGenerator::next() {
        if (next_move > moves.end()) {
            throw std::out_of_range(
                "The generator execeeded allocated space for moves.");
        }

        Move& move = *next_move;
        std::advance(next_move, 1);

        return move;
    }

    std::span<Move> MoveGenerator::next_n(int n) {
        std::span<Move> span(next_move, n);

        if (span.end().base() > moves.end()) {
            throw std::out_of_range(
                "The generator tried to yield more moves than remaining.");
        }

        std::advance(next_move, n);

        return span;
    }

    void MoveGenerator::from_bitboard(Pieces::Piece piece, square from,
                                      bitboard moves, bitboard captures) {

        bitboard::scan(moves, [&](square to) {
            auto& move = next();

            move.piece.moved = piece;

            move.from = from;
            move.to = to;

            if (captures.is_set_at(to)) {
                move.piece.captured = board.piece_at(to);
            }
        });
    }

} // namespace Game::Generators

namespace Game::Generators {
    void initialize_tables() {
        Knights::initialize_table();
        Kings::initialize_table();
        Magic::initialize_magic_tables();
    }

    std::vector<Move> generate_moves(Board board) {
        MoveGenerator generator(board);

        // TODO: Check for partialy pinned pawns
        auto [absolute_pins, partial_pins, pinners] =
            Kings::get_pinned_pieces(board);
        generator.pins = {absolute_pins, partial_pins, pinners};

        generator.enpassant.pinned = Kings::is_enpassant_pinned(board);

        Pawns::gen_pawns_moves(generator);

        Sliders::gen_bishops_moves(generator);
        Sliders::gen_rooks_moves(generator);
        Sliders::gen_queens_moves(generator);

        Knights::gen_knights_moves(generator);

        Kings::gen_king_moves(generator);

        return generator.get_generated();
    }

} // namespace Game::Generators
