#pragma once

#include "game.hpp"
#include <iostream>

namespace Utils {
    inline constexpr Types::bitboard bit_at(Types::square index) {
        return 1ul << index;
    }

    inline constexpr bool is_set_at(Types::square index,
                                    Types::bitboard bitboard) {
        return (bitboard >> index) & 1ul;
    }

    inline constexpr Types::bitboard last_bit(Types::bitboard bitboard) {
        return bitboard & 1;
    }

    inline constexpr Types::bitboard flip_vertically(Types::bitboard value) {
        const uint64_t k1 = 0x00FF00FF00FF00FFul;
        const uint64_t k2 = 0x0000FFFF0000FFFFul;
        value = ((value >> 8) & k1) | ((value & k1) << 8);
        value = ((value >> 16) & k2) | ((value & k2) << 16);
        value = (value >> 32) | (value << 32);
        return value;
    }

    inline constexpr Types::bitboard flip_horizontally(Types::bitboard value) {
        const uint64_t k1 = 0x5555555555555555ul;
        const uint64_t k2 = 0x3333333333333333ul;
        const uint64_t k4 = 0x0f0f0f0f0f0f0f0ful;
        value = ((value >> 1) & k1) + 2 * (value & k1);
        value = ((value >> 2) & k2) + 4 * (value & k2);
        value = ((value >> 4) & k4) + 16 * (value & k4);
        return value;
    }

    inline void print_bitboard(Game::bitboard bboard) {
        for (int rank = 7; rank >= 0; --rank) {
            for (int file = 7; file >= 0; --file) {
                int square_index = rank * 8 + file;
                std::cout << ((bboard >> square_index) & 1ULL ? '1' : '0')
                          << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    inline constexpr uint8_t column(Types::square index) { return index % 8; }

    inline constexpr uint8_t row(Types::square index) { return index / 8; }

    inline constexpr Types::square start_of_row(Types::square index) {
        return index - column(index);
    }

} // namespace Utils
