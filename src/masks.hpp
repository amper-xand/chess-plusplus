#pragma once

#include "game.hpp"
#include "utils.hpp"

namespace Game::Generators::Masks {
    const bitboard fullboard_mask = 0xFFFFFFFFFFFFFFFF;

    const bitboard border_mask = 0xFF818181818181FF;

    const bitboard vertical_moves_mask = 0x0101010101010101;
    const bitboard horizontal_moves_mask = 0xFF;

    //                                                                  8 7 6 5
    //                                                                  4 3 2 1
    const bitboard r_diagonal_moves_mask /* right - left */ =
        0x0102040810204080;
    const bitboard l_diagonal_moves_mask /* left - right */ =
        0x8040201008040201;

    const bitboard vertical_rel_blo_mask =
        vertical_moves_mask & ~0x0100000000000001;
    const bitboard horizontal_rel_blo_mask = horizontal_moves_mask & ~0x81;

    const bitboard diagonal_masks[] = {
        0x0000000000000001, // 0
        0x0000000000000102, // 1
        0x0000000000010204, // 2
        0x0000000001020408, // 3
        0x0000000102040810, // 4
        0x0000010204081020, // 5
        0x0001020408102040, // 6
        /*---------------*/
        0x0102040810204080, // 7
        /*---------------*/
        0x0204081020408000, // 8
        0x0408102040800000, // 9
        0x0810204080000000, // 10
        0x1020408000000000, // 11
        0x2040800000000000, // 12
        0x4080000000000000, // 13
        0x8000000000000000, // 14
    };

    inline bitboard get_diagonal_at(square index) {
        return diagonal_masks[Utils::column(index) + Utils::row(index)];
    }

    inline bitboard get_rev_diagonal_at(square index) {
        return Utils::flip_horizontally(get_diagonal_at(
            Utils::start_of_row(index) + (7 - Utils::column(index))));
    }

    inline bitboard make_n_mask(square index) {
        return Masks::fullboard_mask
               // Moves the mask one above the square
               << (Utils::start_of_row(index) + 8);
    }

    inline bitboard make_s_mask(square index) {
        // Moves one row above the square and gets the trailing zeros
        return Utils::bit_at(Utils::start_of_row(index) + 8) - 1;
    }

    inline bitboard make_e_mask(square index) {
        return (Utils::bit_at(Utils::column(index)) - 1) * 0x0101010101010101UL;
        // return ((0xFF << Utils::column(index)) & 0xFF) *
        // 0x0101010101010101UL;
    }

    inline bitboard make_w_mask(square index) {
        return (~make_e_mask(index)) ^ Utils::bit_at(index) * 0x0101010101010101UL;
        // return ~((0xFE << index % 8) & 0xFF) * 0x0101010101010101UL;
    }

} // namespace Game::Generators::Masks

//                                     //
//  0 0 0 0 0 0 0 1   1 0 0 0 0 0 0 0  //
//  0 0 0 0 0 0 1 0   0 1 0 0 0 0 0 0  //
//  0 0 0 0 0 1 0 0   0 0 1 0 0 0 0 0  //
//  0 0 0 0 1 0 0 0   0 0 0 1 0 0 0 0  //
//  0 0 0 1 0 0 0 0   0 0 0 0 1 0 0 0  //
//  0 0 1 0 0 0 0 0   0 0 0 0 0 1 0 0  //
//  0 1 0 0 0 0 0 0   0 0 0 0 0 0 1 0  //
//  1 0 0 0 0 0 0 0   0 0 0 0 0 0 0 1  //
//                                     //
//  0 0 0 0 0 1 0 0   0 0 1 0 0 0 0 0  //
//  0 0 0 0 1 0 0 0   0 0 0 1 0 0 0 0  //
//  0 0 0 1 0 0 0 0   0 0 0 0 1 0 0 0  //
//  0 0 1 0 0 0 0 0   0 0 0 0 0 1 0 0  //
//  0 1 0 0 0 0 0 0   0 0 0 0 0 0 1 0  //
//  1 0 0 0 0 0 0 0   0 0 0 0 0 0 0 1  //
//  0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0  //
//  0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0  //
//                                     //
//  0 0 0 0 0 0 0 1   0 0 0 0 0 0 0 0  //
//  0 0 0 0 0 0 1 0   0 0 0 0 0 0 0 0  //
//  0 0 0 0 0 1 0 0   0 0 0 0 0 0 0 0  //
//  0 0 0 0 1 0 0 0   0 0 0 0 0 0 0 0  //
//  0 0 0 1 0 0 0 0   0 0 0 0 0 0 0 0  //
//  0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0  //
//  0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0  //
//  0 0 0 0 0 0 0 0   0 0 0 0 0 0 0 0  //
//                                     //
