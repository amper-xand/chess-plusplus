#include "piecewise.hpp"

namespace core::generators::knights {
    bitboard available_moves[64];

    bitboard gen_knight_moves(square index) {
        bitboard moves = 0;

        square col = index.column();
        square row = index.row();

        // Top-most left
        if (row <= 5 && 1 <= col)
            moves |= bitboard::bit_at(index.up(2).right());

        // Bottom-most left
        if (2 <= row && 1 <= col)
            moves |= bitboard::bit_at(index.down(2).right());

        // Top-most right
        if (row <= 5 && col <= 6)
            moves |= bitboard::bit_at(index.up(2).left());

        // Bottom-most right
        if (2 <= row && col <= 6)
            moves |= bitboard::bit_at(index.down(2).left());

        // Top left-most
        if (row <= 6 && 2 <= col)
            moves |= bitboard::bit_at(index.up().right(2));

        // Bottom left-most
        if (1 <= row && 2 <= col)
            moves |= bitboard::bit_at(index.down().right());

        // Top right-most
        if (row <= 6 && col <= 5)
            moves |= bitboard::bit_at(index.up().left(2));

        // Bottom right-most
        if (1 <= row && col <= 5)
            moves |= bitboard::bit_at(index.down().left(2));

        return moves;
    }

    void initialize_table() {
        for (square index = 0; index < 64; ++index) {
            available_moves[index] = gen_knight_moves(index);
        }
    }

    MoveGenerator& gen_knights_moves(MoveGenerator& generator) {
        bitboard knights =
            generator.board.allied(Piece::KNIGHTS).pop(generator.pins.absolute);
        bitboard capturable = generator.board.enemies();
        bitboard blockers = generator.board.allies();

        bitboard::scan(knights, [&](square index) {
            bitboard moves = knights::available_moves[index].pop(blockers);
            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Piece::KNIGHTS, index, moves, captures);
        });

        return generator;
    }

    MoveGenerator& gen_check_blocks(MoveGenerator& generator,
                                    bitboard allowed) {

        auto& board = generator.board;

        bitboard capturable = board.enemies();
        bitboard blockers = generator.board.allies();

        // Pinned pieces cannot move or capture a piece giving check
        bitboard pinned = generator.pins.absolute;

        bitboard knights = board.allied(Piece::KNIGHTS).pop(pinned);

        bitboard::scan(knights, [&](square index) {
            bitboard moves =
                knights::available_moves[index].mask(allowed).pop(blockers);
            bitboard captures = moves.mask(capturable);

            generator.from_bitboard(Piece::KNIGHTS, index, moves, captures);
        });

        return generator;
    }
} // namespace core::generators::knights
