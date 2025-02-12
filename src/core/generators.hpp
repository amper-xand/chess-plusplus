#pragma once

#include "core/types.hpp"
#include <core/representation.hpp>

#include <array>
#include <vector>

namespace core::generators {

    constexpr uint8_t max_moves = 218;

    struct MoveGenerator {
      private:
        std::array<Move, max_moves> moves;
        uint8_t nxt = 0;

      public:
        // clang-format off
        
        // data calculated before generation
        struct { bitboard absolute, partial, pinners; } 
            pins;
        struct { bool pinned = false; }
            enpassant;

        // clang-format on

        const Board& board;

        MoveGenerator(const Board& board) : board(board) {}

        void populate() {
            // save board state to rollback later
            Move base_state = Move::default_mv;
            base_state.ep(board.enpassant.pawn);
            base_state.castle(board.castling.get_state(board.turn));
            
           moves.fill(base_state);

        }

        Move& next() { return moves.at(nxt++); }

        std::vector<Move> generated() {
            return std::vector(moves.begin(), moves.begin() + nxt);
        }

        template <piece_t piece>
        void bulk(square from, bitboard moves, bitboard captures) {
            bitboard::scan(moves, [&](square to) {
                auto& move = next();

                move.moved(piece);

                move.from(from);
                move.to(to);

                if (captures.bit(to))
                    move.captured(board.piece(to));
            });
        }
    };

    void initialize_tables();

    std::vector<Move> generate_moves(const Board& board);
} // namespace core::generators
