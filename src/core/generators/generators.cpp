#include "generators.hpp"

#include "piecewise/piecewise.hpp"

#include "../representation/representation.hpp"
#include "magic/magic.hpp"

#include <climits>
#include <cstdio>
#include <iostream>
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

    void generate_pins(MoveGenerator& generator) {
        bitboard str_pins = Kings::pin_rook_xrays(generator.board);
        bitboard dia_pins = Kings::pin_bishop_xrays(generator.board);

        Board& board = generator.board;

        generator.pins.pinners = board.enemies().mask(str_pins | dia_pins);

        generator.pins.partial = str_pins.mask(board.rooks | board.queens);
        generator.pins.partial |= dia_pins.mask(board.bishops | board.queens);

        generator.pins.absolute = board.allies()
                                      .mask(str_pins | dia_pins)
                                      .pop(generator.pins.partial);

        generator.enpassant.pinned = Kings::is_enpassant_pinned(generator);
    }

    std::vector<Move> check_generation(MoveGenerator& generator,
                                       bitboard checkers) {

        Board& board = generator.board;

        Kings::gen_king_moves<false>(generator);

        // If there is more than one checking piece
        // then just generate the king moves
        if ((checkers & (checkers - 1)) != 0) {
            return generator.get_generated();
        }

        bitboard allowed_squares = checkers;

        // if the piece is not a knight or a pawn
        // then try to block it
        if (checkers.mask(board.knights | board.pawns) != 0) {
            bitboard king = board.allied(Pieces::KINGS);
            square king_pos = king.rzeros();

            // get the slider to the piece direction
            bitboard slider = Magic::Rooks::get_slider(king_pos);

            if ((slider & checkers) == 0) {
                // if the slider did not hit the piece
                // try the other direction
                slider = Magic::Bishops::get_slider(king_pos);
            }

            allowed_squares |=
                bitboard::interval(king, checkers).mask(slider).pop(king);
        }

        Sliders::gen_check_blocks(generator, allowed_squares);
        Knights::gen_check_blocks(generator, allowed_squares);
        Pawns::gen_check_blocks(generator, allowed_squares);

        return generator.get_generated();
    }

    std::vector<Move> generate_moves(Board board) {
        MoveGenerator generator(board);

        generate_pins(generator);

        bitboard checkers = Kings::checking_pieces(board);

        if (checkers != 0) {
            return check_generation(generator, checkers);
        }

        Pawns::gen_pawns_moves(generator);

        Sliders::gen_slider_moves(generator);

        Knights::gen_knights_moves(generator);

        Kings::gen_king_moves(generator);

        return generator.get_generated();
    }

} // namespace Game::Generators
