#include "generation.hpp"

#include <core/magic.hpp>

#include <array>

namespace core {

Move& generation::GenerationContext::next() { return moves.at(nxt++); }

void generation::GenerationContext::bulk(
    Piece moved, square from, bitboard moves, bitboard captures) {
    for (square index = 0; moves != 0; moves >>= 1, captures >>= 1, ++index) {
        if (moves[0]) {
            Move& move = this->next();

            move.moved = moved;

            move.from = from;
            move.to = index;

            if (captures[0]) {
                move.target = board.piece(move.to);
            }
        }
    }
}

std::vector<Move> generation::GenerationContext::get_generated_moves() {
    return std::vector(moves.begin(), moves.begin() + nxt);
}

std::vector<Move> generation::generate_moves(const Board& board) {
    GenerationContext context(board);

    generation::generate_pawn_moves(context);
    generation::generate_knight_moves(context);
    generation::generate_rook_moves(context);
    generation::generate_bishop_moves(context);

    return context.get_generated_moves();
}

void generation::generate_pawn_moves(GenerationContext& context) {
    auto& board = context.board;

    bitboard pawns = board.allied(Piece::PAWNS);

    bitboard capturable = board.enemies();
    bitboard blockers = board.all();

    // Advance the pawns then remove those who were blocked

    bitboard single_advances = pawns.forward(board.turn, 8);
    single_advances = single_advances.exclude(blockers);

    bitboard double_advances =
        single_advances
            // advance pawns that advanced from second row
            .mask(board.turn.isWhite() ? bitboard::masks::rank(2)
                                       : bitboard::masks::rank(5))
            .forward(board.turn, 8);
    double_advances = double_advances.exclude(blockers);

    // Advance the pawns to a capture position,
    bitboard l_captures =
        pawns.exclude(bitboard::masks::file(7)).forward(board.turn, 8) << 1;
    bitboard r_captures =
        pawns.exclude(bitboard::masks::file(0)).forward(board.turn, 8) >> 1;

    // keep the pawns that are over an enemy piece
    l_captures = l_captures.mask(capturable);
    r_captures = r_captures.mask(capturable);

    for (square index = 0; index < 64;                 //
        single_advances >>= 1, double_advances >>= 1,  //
        l_captures >>= 1, r_captures >>= 1, index++) {
        // exit the loop when the bitboards are empty
        if ((single_advances | double_advances | l_captures | r_captures) == 0)
            break;

        if (single_advances[0]) {
            Move& m = context.next();

            m.moved = Piece::PAWNS;
            m.from = board.turn.isWhite() ? index.down() : index.up();
            m.to = index;
        }

        if (double_advances[0]) {
            Move& m = context.next();

            m.moved = Piece::PAWNS;
            m.from = board.turn.isWhite() ? index.down(2) : index.up(2);
            m.to = index;
        }

        if (l_captures[0]) {
            Move& m = context.next();

            m.moved = Piece::PAWNS;
            m.from = (board.turn.isWhite() ? index.down() : index.up()).right();
            m.to = index;
            m.target = board.piece(m.to);
        }

        if (r_captures[0]) {
            Move& m = context.next();

            m.moved = Piece::PAWNS;
            m.from = (board.turn.isWhite() ? index.down() : index.up()).left();
            m.to = index;
            m.target = board.piece(m.to);
        }
    }
}

consteval std::array<bitboard, 64> intialize_knight_table() {
    std::array<bitboard, 64> moves = {0};

    for (square index = 0; index < 64; ++index) {
        uint8_t row = index.row();
        uint8_t column = index.column();

        auto set_move = [&](int row, int col) {
            if (row >= 0 && row < 8 && col >= 0 && col < 8)
                moves[index] |= square::at(row, col).bb();
        };

        set_move(row + 2, column + 1);
        set_move(row + 1, column + 2);
        set_move(row - 1, column + 2);
        set_move(row - 2, column + 1);
        set_move(row - 2, column - 1);
        set_move(row - 1, column - 2);
        set_move(row + 1, column - 2);
        set_move(row + 2, column - 1);
    }

    return moves;
};

constexpr auto knights_moves = intialize_knight_table();

void generation::generate_knight_moves(GenerationContext& context) {
    auto& board = context.board;

    bitboard knights = board.allied(Piece::KNIGHTS);

    bitboard capturable = board.enemies();
    bitboard blockers = board.allies();

    for (square index = 0; knights != 0; ++index, knights >>= 1) {
        if (knights[0]) {
            context.bulk(Piece::KNIGHTS, index,
                knights_moves[index].exclude(blockers), capturable);
        }
    }
}

void generation::generate_rook_moves(generation::GenerationContext& context) {
    auto& board = context.board;

    bitboard rooks = board.allied(Piece::ROOKS);

    bitboard capturable = board.enemies();
    bitboard blockers = board.all();

    for (square index = 0; rooks != 0; ++index, rooks >>= 1) {
        if (rooks[0]) {
            bitboard moves = magic::rooks::get_avail_moves(blockers, index);
            moves = moves.exclude(blockers ^ capturable);

            bitboard captures = moves.mask(capturable);

            context.bulk(Piece::ROOKS, index, moves, captures);
        }
    }
}

void generation::generate_bishop_moves(generation::GenerationContext& context) {
    auto& board = context.board;

    bitboard bishops = board.allied(Piece::BISHOPS);

    bitboard capturable = board.enemies();
    bitboard blockers = board.all();

    for (square index = 0; bishops != 0; ++index, bishops >>= 1) {
        if (bishops[0]) {
            bitboard moves = magic::bishops::get_avail_moves(blockers, index);
            moves = moves.exclude(blockers ^ capturable);

            bitboard captures = moves.mask(capturable);

            context.bulk(Piece::BISHOPS, index, moves, captures);
        }
    }
}

}  // namespace core
