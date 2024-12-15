#include "piecewise.hpp"

#include <cstdio>

namespace Game::Generators::Pawns {
    // clang-format off
    bitboard get_advances(bitboard pawns, bitboard blockers, bool color) {

        if (color == Color::WHITE) pawns <<= 8;
        else                       pawns >>= 8;

        return pawns.pop(blockers);
    }

    bitboard get_advances(bitboard pawns, bitboard blockers, Color color) {
        return get_advances(pawns, blockers, (bool) color);
    }

    bitboard east_attacks(bitboard pawns, bool color) {
        // Filter pawns that will wrap around the board
        bitboard attacks = pawns.pop(0x0101010101010101);

        if (color == Color::WHITE) {
            attacks <<= 7; // up + right
        } else {
            attacks >>= 9; // down + right
        }

        return attacks;
    }

    bitboard east_attacks(bitboard pawns, Color color) {
        return east_attacks(pawns, (bool) color);
    }

    bitboard west_attacks(bitboard pawns, bool color) {
        // Filter pawns that will wrap around the board
        bitboard attacks = pawns.pop(0x8080808080808080);

        if (color == Color::WHITE) {
            attacks <<= 9; // up + left
        } else {
            attacks >>= 7; // down + left
        }

        return attacks;
    }

    bitboard west_attacks(bitboard pawns, Color color) {
        return west_attacks(pawns, (bool) color);
    }
    // clang-format on

    void gen_promotion(MoveGenerator& generator, Move& move) {
        if (!(move.to.row() == 0 || move.to.row() == 7))
            return;

        move.promotion = Piece::QUEENS;

        for (auto promotion : {Piece::KNIGHTS, Piece::BISHOPS, Piece::ROOKS}) {

            generator.next().same_as(move).promotion = promotion;
        }
    };

    void gen_from_advances(MoveGenerator& generator, bitboard singles,
                           bitboard doubles) {

        bitboard::scan(singles, [&](square index) {
            Move& move = generator.next();
            move.piece.moved = Piece::PAWNS;

            move.from = index;
            move.to = generator.board.turn ? index.up() : index.down();

            // Mark ep as rejected
            move.enpassant.captured = generator.board.enpassant.available;

            gen_promotion(generator, move);
        });

        bitboard::scan(doubles, [&](square index) {
            Move& move = generator.next();
            move.piece.moved = Piece::PAWNS;

            move.enpassant.set = true;

            move.from = index;
            move.to = generator.board.turn ? index.up(2) : index.down(2);

            // Mark ep as rejected
            move.enpassant.captured = generator.board.enpassant.available;
        });
    }

    void gen_from_captures(MoveGenerator& generator, bitboard west,
                           bitboard east) {

        bitboard::scan(east, [&](square index) {
            Move& move = generator.next();
            move.piece.moved = Piece::PAWNS;

            move.to = index;
            move.from =
                generator.board.turn ? index.down().left() : index.up().left();

            move.piece.captured = generator.board.piece_at(index);

            // Mark ep as rejected
            move.enpassant.captured = generator.board.enpassant.available;

            gen_promotion(generator, move);
        });

        bitboard::scan(west, [&](square index) {
            Move& move = generator.next();
            move.piece.moved = Piece::PAWNS;

            move.to = index;
            move.from = generator.board.turn ? index.down().right()
                                             : index.down().right();

            move.piece.captured = generator.board.piece_at(index);

            // Mark ep as rejected
            move.enpassant.captured = generator.board.enpassant.available;

            gen_promotion(generator, move);
        });
    }

    void gen_enpassant(MoveGenerator& generator, bitboard west, bitboard east) {

        if (east != 0) {
            square index = east.rzeros();

            Move& move = generator.next();
            move.piece.moved = Piece::PAWNS;

            move.enpassant.take = true;
            move.enpassant.captured = generator.board.enpassant.capturable;

            move.to = index;
            move.from =
                generator.board.turn ? index.down().left() : index.up().left();

            move.piece.captured = generator.board.piece_at(index);
        };

        if (west != 0) {
            square index = west.rzeros();

            Move& move = generator.next();
            move.piece.moved = Piece::PAWNS;

            move.enpassant.take = true;
            move.enpassant.captured = generator.board.enpassant.capturable;

            move.to = index;
            move.from = generator.board.turn ? index.down().right()
                                             : index.up().right();

            move.piece.captured = generator.board.piece_at(index);
        };
    }

    MoveGenerator& gen_pawns_moves(MoveGenerator& generator) {
        Board& board = generator.board;

        bitboard blockers = board.all();

        bitboard pawns = board.allied(Piece::PAWNS)
                         // Remove pinned pawns
                         & ~generator.pins.absolute;

        struct {
            bitboard singles, doubles;
        } advances;

        advances.singles = Pawns::get_advances(
            // Partial pins can only capture
            pawns.mask(~generator.pins.partial), blockers, board.turn);

        advances.doubles =
            advances.singles.mask
            // If a pawn advanced to the 3rd or 6th rank, advance it again
            (board.turn ? 0x0000000000FF0000 : 0x0000FF0000000000);

        advances.doubles =
            Pawns::get_advances(advances.doubles, blockers, board.turn);

        // return pawns to their original positions
        if (board.turn) {
            advances.singles >>= 8;
            advances.doubles >>= 16;
        } else {
            advances.singles <<= 8;
            advances.doubles <<= 16;
        }

        struct {
            bitboard east = 0, west = 0;
        } attacks, captures, enpassant;

        square king_position = board.allied(Piece::KINGS).rzeros();

        bitboard east_partial = bitboard::Masks::make_e_mask(king_position)
                                    .mask(generator.pins.partial);

        bitboard west_partial = bitboard::Masks::make_w_mask(king_position)
                                    .mask(generator.pins.partial);

        attacks.east = Pawns::east_attacks(
            // The west partially pinned pawns cannot make east captures
            pawns.mask(~west_partial), board.turn);

        attacks.west = Pawns::west_attacks(
            // The east partially pinned pawns cannot make west captures
            pawns.mask(~east_partial), board.turn);

        captures.east = board.enemies().mask(attacks.east);
        captures.west = board.enemies().mask(attacks.west);

        if (board.enpassant.available && !generator.enpassant.pinned) {
            enpassant.east =
                attacks.east.mask(bitboard::bit_at(board.enpassant.tail));
            enpassant.west =
                attacks.west.mask(bitboard::bit_at(board.enpassant.tail));
        }

        gen_from_advances(generator, advances.singles, advances.doubles);
        gen_from_captures(generator, captures.west, captures.east);
        gen_enpassant(generator, enpassant.west, enpassant.east);

        return generator;
    }

    MoveGenerator& gen_check_blocks(MoveGenerator& generator,
                                    bitboard allowed) {

        auto& board = generator.board;

        bitboard pawns =
            board.allied(Piece::PAWNS)
                .pop(generator.pins.absolute | generator.pins.partial);

        bitboard blockers = board.all();

        bitboard singles = get_advances(pawns, blockers, board.turn);
        bitboard doubles = get_advances(singles, blockers, board.turn);

        gen_from_advances(generator, //
                          singles.mask(allowed), doubles.mask(allowed));

        bitboard capturable = board.enemies().mask(allowed);

        bitboard attacks_east = east_attacks(pawns, board.turn);
        bitboard attacks_west = west_attacks(pawns, board.turn);

        bitboard captures_east = attacks_east.mask(capturable);
        bitboard captures_west = attacks_west.mask(capturable);

        gen_from_captures(generator, captures_west, captures_east);

        if (
            // if there is only one place to block
            (allowed & (allowed - 1)) == 0  //
            && (allowed & board.pawns) != 0 // and it is a pawn
            && board.enpassant.available    // and en passant is available
        ) {
            // then check if a pawn can stop the check with en passant
            bitboard east_ep =
                attacks_east.mask(bitboard::bit_at(board.enpassant.tail));

            bitboard west_ep =
                attacks_west.mask(bitboard::bit_at(board.enpassant.tail));

            gen_enpassant(generator, west_ep, east_ep);
        }

        return generator;
    }

} // namespace Game::Generators::Pawns
