#pragma once

#include "../core/representation/representation.hpp"
#include <cstdint>
#include <iostream>

namespace Utils {
    inline constexpr Game::bitboard bit_at(Game::square index) {
        return 1ul << index;
    }

    inline constexpr bool is_set_at(Game::square index,
                                    Game::bitboard bitboard) {
        return (bitboard >> index) & 1ul;
    }

    inline constexpr Game::bitboard last_bit(Game::bitboard bitboard) {
        return bitboard & 1;
    }

    inline constexpr Game::bitboard flip_vertically(Game::bitboard value) {
        const uint64_t k1 = 0x00FF00FF00FF00FFul;
        const uint64_t k2 = 0x0000FFFF0000FFFFul;
        value = ((value >> 8) & k1) | ((value & k1) << 8);
        value = ((value >> 16) & k2) | ((value & k2) << 16);
        value = (value >> 32) | (value << 32);
        return value;
    }

    inline constexpr Game::bitboard flip_horizontally(Game::bitboard value) {
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

    inline constexpr uint8_t column(Game::square index) { return index % 8; }

    inline constexpr uint8_t row(Game::square index) { return index / 8; }

    inline constexpr Game::square index_at(uint8_t row, uint8_t col) {
        return col + 8 * row;
    }

    inline constexpr Game::square start_of_row(Game::square index) {
        return index - column(index);
    }

} // namespace Utils
