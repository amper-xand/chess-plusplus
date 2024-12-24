#include "piecewise.hpp"

#include "../magic/magic.hpp"

namespace core::generators::sliders {

    template <const auto& sld_moves, piece_t slider>
    MoveGenerator& gen_slider(MoveGenerator& generator, bitboard king_slider) {
        auto& board = generator.board;

        bitboard pieces = board.allied(slider), blockers = board.all(),
                 noncaptures = board.allies();

        // Remove absolutely pinned pieces
        pieces &= ~generator.pins.absolute;

        uint8_t piece_count = pieces.popcount();

        bitboard available_per_piece[piece_count];
        square piece_positions[piece_count];

        uint8_t current_piece = 0;

        bitboard::scan(pieces, [&](square index) {
            bitboard available_moves =
                sld_moves(blockers, index) & ~noncaptures;

            piece_positions[current_piece] = index;

            available_per_piece[current_piece] = available_moves;

            ++current_piece;
        });

        for (uint8_t current_piece = 0; current_piece < piece_count;
             ++current_piece) {

            if (generator.pins
                    .partial
                    // Check if piece is partially pinned
                    .is_set_at(piece_positions[current_piece])) {

                // Only allow the partially pinned piece
                // to move colinearly to the king and the pinners
                // based on the slider
                available_per_piece[current_piece] &= king_slider;
            }

            bitboard captures =
                available_per_piece[current_piece].mask(board.enemies());

            generator.from_bitboard(slider, piece_positions[current_piece],
                                    available_per_piece[current_piece],
                                    captures);
        }

        return generator;
    }

    MoveGenerator& gen_slider_moves(MoveGenerator& generator) {
        square king_position = generator.board.allied(Piece::KINGS).rzeros();

        bitboard king_b_sld = magic::bishops::get_slider(king_position);
        bitboard king_r_sld = magic::rooks::get_slider(king_position);

        gen_slider<magic::bishops::get_avail_moves, Piece::BISHOPS> //
            (generator, king_b_sld);

        gen_slider<magic::rooks::get_avail_moves, Piece::ROOKS> //
            (generator, king_r_sld);

        gen_slider<magic::bishops::get_avail_moves, Piece::QUEENS> //
            (generator, king_b_sld);

        gen_slider<magic::rooks::get_avail_moves, Piece::QUEENS> //
            (generator, king_r_sld);

        return generator;
    }

    MoveGenerator& gen_check_blocks(MoveGenerator& generator,
                                    bitboard allowed) {

        auto& board = generator.board;

        bitboard blockers = board.all();
        bitboard capturable = board.enemies();

        // Pinned pieces cannot move or capture a piece giving check
        bitboard pinned = generator.pins.absolute | generator.pins.partial;

        // clang-format off
        bitboard::scan(board.allied(Piece::BISHOPS).pop(pinned), [&](square index) {
            bitboard moves =
                magic::bishops::get_avail_moves(blockers, index).mask(allowed);
            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Piece::BISHOPS, index, moves, captures);
        });

        bitboard::scan(board.allied(Piece::ROOKS).pop(pinned), [&](square index) {
            bitboard moves =
                magic::rooks::get_avail_moves(blockers, index).mask(allowed);
            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Piece::ROOKS, index, moves, captures);
        });

        bitboard::scan(board.allied(Piece::QUEENS).pop(pinned), [&](square index) {
            bitboard moves =
                magic::rooks::get_avail_moves(blockers, index)
                    .join(magic::bishops::get_avail_moves(blockers, index))
                    .mask(allowed);

            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Piece::QUEENS, index, moves, captures);
        });
        // clang-format on

        return generator;
    }
} // namespace core::generators::sliders
