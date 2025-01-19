#include "generators.hpp"
#include "core/types.hpp"

#include <core/generators/piecewise.hpp>
#include <core/magic/magic.hpp>
#include <core/representation.hpp>

#include <vector>

namespace core::generators {} // namespace core::generators

namespace core::generators {
    void initialize_tables() {
        knights::initialize_table();
        kings::initialize_table();
        magic::initialize_magic_tables();
    }

    void generate_pins(MoveGenerator& generator) {
        bitboard str_pins = kings::pin_rook_xrays(generator.board);
        bitboard dia_pins = kings::pin_bishop_xrays(generator.board);

        const Board& board = generator.board;

        generator.pins.pinners = board.enemies().mask(str_pins | dia_pins);

        generator.pins.partial = str_pins.mask(board.rooks | board.queens);
        generator.pins.partial |= dia_pins.mask(board.bishops | board.queens);

        generator.pins.absolute = board.allies()
                                      .mask(str_pins | dia_pins)
                                      .pop(generator.pins.partial);

        generator.enpassant.pinned = kings::is_enpassant_pinned(generator);
    }

    std::vector<Move> check_generation(MoveGenerator& generator,
                                       bitboard checkers) {

        const Board& board = generator.board;

        kings::gen_king_moves<false>(generator);

        // If there is more than one checking piece
        // then just generate the king moves
        if ((checkers & (checkers - 1)) != 0) {
            return generator.generated();
        }

        bitboard allowed_squares = checkers;

        // if the piece is not a knight or a pawn
        // then try to block it
        if (checkers.mask(board.knights | board.pawns) != 0) {
            bitboard king = board.allied(Piece::KINGS);
            square king_pos = king.rzeros();

            // get the slider to the piece direction
            bitboard slider = magic::rooks::get_slider(king_pos);

            if ((slider & checkers) == 0) {
                // if the slider did not hit the piece
                // try the other direction
                slider = magic::bishops::get_slider(king_pos);
            }

            allowed_squares |=
                bitboard::interval(king, checkers).mask(slider).pop(king);
        }

        sliders::gen_check_blocks(generator, allowed_squares);
        knights::gen_check_blocks(generator, allowed_squares);
        pawns::gen_check_blocks(generator, allowed_squares);

        return generator.generated();
    }

    std::vector<Move> generate_moves(const Board& board) {
        MoveGenerator generator(board);

        generator.populate();

        generate_pins(generator);

        bitboard checkers = kings::checking_pieces(board);

        if (checkers != 0) {
            return check_generation(generator, checkers);
        }

        pawns::gen_pawns_moves(generator);

        sliders::gen_slider_moves(generator);

        knights::gen_knights_moves(generator);

        kings::gen_king_moves(generator);

        return generator.generated();
    }

} // namespace core::generators
