#include "piecewise.hpp"

#include "../magic/magic.hpp"

namespace Game::Generators::Sliders {

    template <const auto sld_moves, Pieces::Piece slider>
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
        square king_position = generator.board.allied(Pieces::KINGS).rzeros();

        bitboard king_b_sld = Magic::Bishops::get_slider(king_position);
        bitboard king_r_sld = Magic::Rooks::get_slider(king_position);

        gen_slider<Magic::Bishops::get_avail_moves, Pieces::BISHOPS> //
            (generator, king_b_sld);

        gen_slider<Magic::Rooks::get_avail_moves, Pieces::ROOKS> //
            (generator, king_r_sld);

        gen_slider<Magic::Bishops::get_avail_moves, Pieces::QUEENS> //
            (generator, king_b_sld);

        gen_slider<Magic::Rooks::get_avail_moves, Pieces::QUEENS> //
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
        bitboard::scan(board.allied(Pieces::BISHOPS).pop(pinned), [&](square index) {
            bitboard moves =
                Magic::Bishops::get_avail_moves(blockers, index).mask(allowed);
            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Pieces::BISHOPS, index, moves, captures);
        });

        bitboard::scan(board.allied(Pieces::ROOKS).pop(pinned), [&](square index) {
            bitboard moves =
                Magic::Rooks::get_avail_moves(blockers, index).mask(allowed);
            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Pieces::ROOKS, index, moves, captures);
        });

        bitboard::scan(board.allied(Pieces::QUEENS).pop(pinned), [&](square index) {
            bitboard moves =
                Magic::Rooks::get_avail_moves(blockers, index)
                    .join(Magic::Bishops::get_avail_moves(blockers, index))
                    .mask(allowed);

            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Pieces::QUEENS, index, moves, captures);
        });
        // clang-format on

        return generator;
    }
} // namespace Game::Generators::Sliders
