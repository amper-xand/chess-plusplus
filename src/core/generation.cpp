#include "generation.hpp"

#include <array>

#include "core/types.hpp"

namespace core {

Move& generation::GenerationContext::next() { return moves.at(nxt++); }

std::vector<Move> generation::GenerationContext::get_generated_moves() {
    return std::vector(moves.begin(), moves.begin() + nxt);
}

std::vector<Move> generation::generate_moves(const Board& board) {
    GenerationContext context(board);

    generation::generate_pawn_moves(context);

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

}  // namespace core
