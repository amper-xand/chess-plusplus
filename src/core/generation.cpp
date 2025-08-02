#include <core/generation.hpp>

#include <core/magic.hpp>

#include <array>
#include <bit>

namespace core {

/***************************# Main Generation Path #***************************/

//
Move& generation::GenerationContext::next() { return moves.at(nxt++); }

void generation::GenerationContext::bulk(
    Piece moved, square from, bitboard moves, bitboard captures) {
    for (; moves != 0; moves ^= moves.LSB()) {
        square index = std::countr_zero((bitboard_t)moves);

        Move& move = this->next();

        move.moved = moved;

        move.from = from;
        move.to = index;

        if (captures[index]) {
            move.target = board.piece(move.to);
        }
    }
}

std::vector<Move> generation::GenerationContext::get_generated_moves() {
    return std::vector(moves.begin(), moves.begin() + nxt);
}

std::vector<Move> generation::generate_moves(const Board& board) {
    GenerationContext context(board);

    context.attacked_squares =
        generation::generate_bitboard_squares_attacked(context);

    generation::generate_bitboard_pieces_pinned(
        context, context.pinned.absolute, context.pinned.partial);

    generation::generate_moves_pawn(context);
    generation::generate_moves_knight(context);
    generation::generate_moves_rook(context);
    generation::generate_moves_bishop(context);
    generation::generate_moves_queen(context);
    generation::generate_moves_king(context);

    return context.get_generated_moves();
}

/***************************# Pawn Move Generation #***************************/

//
void generation::generate_moves_pawn(GenerationContext& context) {
    auto& board = context.board;

    bitboard pawns = board.allied(Piece::PAWNS);

    bitboard pawns_pinned = pawns.mask(context.pinned.absolute);

    pawns = pawns ^ pawns_pinned;

    bitboard capturable = board.enemies();
    bitboard blockers = board.all();

    // Advance the pawns then remove those who were blocked
    bitboard advances_single = pawns;
    advances_single = advances_single.forward(board.turn, 8);
    advances_single = advances_single.exclude(blockers);

    bitboard pawns_can_advance_again = board.turn.isWhite()
        ? bitboard::masks::rank(2)
        : bitboard::masks::rank(5);

    // Advance the pawns that were not blocked
    bitboard advances_double = advances_single.mask(pawns_can_advance_again);
    advances_double = advances_double.forward(board.turn, 8);
    advances_double = advances_double.exclude(blockers);

    // Advance the pawns to a capture position

    // Add pinned pawns that can capture
    // FIXME: pawns can capture along the diagonal not just in front of the king
    bitboard king_front = board.allied(Piece::KINGS).forward(board.turn, 8);

    square iking = std::countr_zero((bitboard_t)board.allied(Piece::KINGS));

    bitboard side_right =
        bitboard::masks::file(iking.column()) - bitboard::masks::file(0);
    bitboard side_left = ~side_right;

    bitboard slider = magic::bishops::get_slider(iking);
    side_right &= slider;
    side_left &= slider;

    bitboard capture_direction = board.allied(Piece::KINGS) - 1;

    if (board.turn.isWhite()) capture_direction = ~capture_direction;

    side_left &= capture_direction;
    side_right &= capture_direction;

    bitboard pinned_left = pawns_pinned.mask(side_left);
    bitboard pinned_right = pawns_pinned.mask(side_right);

    bitboard captures_left = pawns | pinned_left;
    bitboard captures_right = pawns | pinned_right;

    // Remove pawns that will overflow
    captures_left = captures_left.exclude(bitboard::masks::file(7));
    captures_right = captures_right.exclude(bitboard::masks::file(0));

    // Move pawns to capture
    captures_left = captures_left.forward(board.turn, 8) << 1;
    captures_right = captures_right.forward(board.turn, 8) >> 1;

    // keep the pawns that are over an enemy piece
    captures_left = captures_left.mask(capturable);
    captures_right = captures_right.mask(capturable);

    for (; advances_single != 0; advances_single ^= advances_single.LSB()) {
        square index = std::countr_zero((bitboard_t)advances_single);

        Move& m = context.next();

        m.moved = Piece::PAWNS;
        m.from = board.turn.isWhite() ? index.down() : index.up();
        m.to = index;
    }

    for (; advances_double != 0; advances_double ^= advances_double.LSB()) {
        square index = std::countr_zero((bitboard_t)advances_double);

        Move& m = context.next();

        m.moved = Piece::PAWNS;
        m.from = board.turn.isWhite() ? index.down() : index.up();
        m.to = index;
    }

    for (; captures_left != 0; captures_left ^= captures_left.LSB()) {
        square index = std::countr_zero((bitboard_t)captures_left);

        Move& m = context.next();

        m.moved = Piece::PAWNS;
        m.from = (board.turn.isWhite() ? index.down() : index.up()).right();
        m.to = index;
        m.target = board.piece(m.to);
    }

    for (; captures_right != 0; captures_right ^= captures_right.LSB()) {
        square index = std::countr_zero((bitboard_t)captures_right);

        Move& m = context.next();

        m.moved = Piece::PAWNS;
        m.from = (board.turn.isWhite() ? index.down() : index.up()).left();
        m.to = index;
        m.target = board.piece(m.to);
    }
}

/**************************# Knight Move Generation #**************************/

//
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

void generation::generate_moves_knight(GenerationContext& context) {
    auto& board = context.board;

    bitboard knights = board.allied(Piece::KNIGHTS);

    bitboard capturable = board.enemies();
    bitboard blockers = board.allies();

    for (; knights != 0; knights ^= knights.LSB()) {
        square index = std::countr_zero((bitboard_t)knights);

        context.bulk(Piece::KNIGHTS, index,
            knights_moves[index].exclude(blockers), capturable);
    }
}

/**************************# Slider Move Generation #**************************/

//
void generation::generate_moves_rook(GenerationContext& context) {
    auto& board = context.board;

    bitboard rooks =
        board.allied(Piece::ROOKS)
            .exclude(context.pinned.absolute | context.pinned.partial);

    bitboard capturable = board.enemies();
    bitboard blockers = board.all();

    for (; rooks != 0; rooks ^= rooks.LSB()) {
        square index = std::countr_zero((bitboard_t)rooks);

        bitboard moves = magic::rooks::get_avail_moves(blockers, index);
        moves = moves.exclude(blockers ^ capturable);

        bitboard captures = moves.mask(capturable);

        context.bulk(Piece::ROOKS, index, moves, captures);
    }

    if (board.allied(Piece::KINGS) == 0) return;

    bitboard partially_pinned = context.pinned.partial.mask(board.rooks);

    square iking = std::countr_zero((bitboard_t)board.allied(Piece::KINGS));
    bitboard king_slider = magic::rooks::get_slider(iking);

    for (; partially_pinned != 0; partially_pinned ^= partially_pinned.LSB()) {
        square index = std::countr_zero((bitboard_t)partially_pinned);

        bitboard moves = magic::rooks::get_avail_moves(blockers, index);
        moves = moves.mask(king_slider);

        bitboard captures = moves.mask(capturable);

        context.bulk(Piece::ROOKS, index, moves, captures);
    }
}

void generation::generate_moves_bishop(GenerationContext& context) {
    auto& board = context.board;

    bitboard bishops =
        board.allied(Piece::BISHOPS)
            .exclude(context.pinned.absolute | context.pinned.partial);

    bitboard capturable = board.enemies();
    bitboard blockers = board.all();

    for (; bishops != 0; bishops ^= bishops.LSB()) {
        square index = std::countr_zero((bitboard_t)bishops);

        bitboard moves = magic::bishops::get_avail_moves(blockers, index);
        moves = moves.exclude(blockers ^ capturable);

        bitboard captures = moves.mask(capturable);

        context.bulk(Piece::BISHOPS, index, moves, captures);
    }

    if (board.allied(Piece::KINGS) == 0) return;

    bitboard partially_pinned = context.pinned.partial.mask(board.bishops);

    square iking = std::countr_zero((bitboard_t)board.allied(Piece::KINGS));
    bitboard king_slider = magic::bishops::get_slider(iking);

    for (; partially_pinned != 0; partially_pinned ^= partially_pinned.LSB()) {
        square index = std::countr_zero((bitboard_t)partially_pinned);

        bitboard moves = magic::bishops::get_avail_moves(blockers, index);
        moves = moves.mask(king_slider);

        bitboard captures = moves.mask(capturable);

        context.bulk(Piece::BISHOPS, index, moves, captures);
    }
}

void generation::generate_moves_queen(GenerationContext& context) {
    auto& board = context.board;

    bitboard queens =
        board.allied(Piece::QUEENS)
            .exclude(context.pinned.absolute | context.pinned.partial);

    bitboard capturable = board.enemies();
    bitboard blockers = board.all();

    for (; queens != 0; queens ^= queens.LSB()) {
        square index = std::countr_zero((bitboard_t)queens);

        bitboard moves =  //
            magic::rooks::get_avail_moves(blockers, index) |
            magic::rooks::get_avail_moves(blockers, index);

        moves = moves.exclude(blockers ^ capturable);

        bitboard captures = moves.mask(capturable);

        context.bulk(Piece::QUEENS, index, moves, captures);
    }

    if (board.allied(Piece::KINGS) == 0) return;

    square iking = std::countr_zero((bitboard_t)board.allied(Piece::KINGS));

    bitboard diagonal_pins = context.pinned.partial.mask(board.queens)
                                 .mask(magic::bishops::get_slider(iking));

    bitboard king_slider = magic::bishops::get_slider(iking);

    for (; diagonal_pins != 0; diagonal_pins ^= diagonal_pins.LSB()) {
        square index = std::countr_zero((bitboard_t)diagonal_pins);

        bitboard moves =
            magic::bishops::get_avail_moves(blockers, index).mask(king_slider);

        bitboard captures = moves.mask(capturable);

        context.bulk(Piece::QUEENS, index, moves, captures);
    }

    bitboard straight_pins = context.pinned.partial.mask(board.queens)
                                 .mask(magic::rooks::get_slider(iking));

    king_slider = magic::rooks::get_slider(iking);

    for (; straight_pins != 0; straight_pins ^= straight_pins.LSB()) {
        square index = std::countr_zero((bitboard_t)straight_pins);

        bitboard moves =
            magic::rooks::get_avail_moves(blockers, index).mask(king_slider);

        bitboard captures = moves.mask(capturable);

        context.bulk(Piece::QUEENS, index, moves, captures);
    }
}

/***************************# King Move Generation #***************************/

//
consteval std::array<bitboard, 64> intialize_king_table() {
    std::array<bitboard, 64> moves = {0};

    for (square index = 0; index < 64; ++index) {
        uint8_t row = index.row();
        uint8_t column = index.column();

        auto set_move = [&](int row, int col) {
            if (row >= 0 && row < 8 && col >= 0 && col < 8)
                moves[index] |= square::at(row, col).bb();
        };

        set_move(row + 1, column + 1);
        set_move(row + 1, column);
        set_move(row + 1, column - 1);

        set_move(row, column + 1);
        set_move(row, column - 1);

        set_move(row - 1, column + 1);
        set_move(row - 1, column);
        set_move(row - 1, column - 1);
    }

    return moves;
}

constexpr auto king_moves = intialize_king_table();

void generation::generate_moves_king(GenerationContext& context) {
    auto& board = context.board;

    bitboard king = board.allied(Piece::KINGS);

    // some test positions don't have a king
    if (!king) return;

    bitboard capturable = board.enemies();
    bitboard blockers = board.allies();

    square index = std::countr_zero((bitboard_t)king);

    bitboard moves =
        king_moves[index].exclude(blockers | context.attacked_squares);
    bitboard captures = moves.mask(capturable);

    context.bulk(Piece::KINGS, index, moves, captures);
}

/***********************# Context Bitboards Generation #***********************/

//
bitboard generation::generate_bitboard_squares_attacked(
    GenerationContext& context) {
    auto board = context.board;

    bitboard attacked_squares = 0;

    struct {
        bitboard diagonal_sliders;
        bitboard straight_sliders;
        bitboard knights;
    } attackers;

    attackers.diagonal_sliders =
        board.enemies().mask(board.bishops | board.queens);

    attackers.straight_sliders =
        board.enemies().mask(board.rooks | board.queens);

    attackers.knights = board.enemy(Piece::KNIGHTS);

    bitboard blockers = board.all().exclude(board.allied(Piece::KINGS));

    for (; attackers.diagonal_sliders != 0;
        attackers.diagonal_sliders ^= attackers.diagonal_sliders.LSB()) {
        square index = std::countr_zero((bitboard_t)attackers.diagonal_sliders);

        attacked_squares |= magic::bishops::get_avail_moves(blockers, index);
    }

    for (; attackers.straight_sliders != 0;
        attackers.straight_sliders ^= attackers.straight_sliders.LSB()) {
        square index = std::countr_zero((bitboard_t)attackers.straight_sliders);

        attacked_squares |= magic::rooks::get_avail_moves(blockers, index);
    }

    for (; attackers.knights != 0;
        attackers.knights ^= attackers.knights.LSB()) {
        square index = std::countr_zero((bitboard_t)attackers.knights);

        attacked_squares |= knights_moves[index];
    }

    bitboard pawns = board.enemy(Piece::PAWNS);

    // left pawn attacks
    attacked_squares |=
        pawns.exclude(bitboard::masks::file(7)).forward(!board.turn, 8) << 1;

    // right pawn attacks
    attacked_squares |=
        pawns.exclude(bitboard::masks::file(0)).forward(!board.turn, 8) >> 1;

    if (board.enemy(Piece::KINGS) != 0)
        attacked_squares |=
            king_moves[std::countr_zero((bitboard_t)board.enemy(Piece::KINGS))];

    return attacked_squares;
}

void generation::generate_bitboard_pieces_pinned(
    GenerationContext& context, bitboard& absolute, bitboard& partial) {
    auto& board = context.board;

    if (!board.allied(Piece::KINGS)) return;

    bitboard pinned = 0;

    bitboard king = board.allied(Piece::KINGS);

    bitboard blockers = board.all();

    bitboard candidates;
    bitboard pinners;

    square iking = std::countr_zero((bitboard_t)king);

    // get the pieces that are block the king

    candidates =
        magic::bishops::get_avail_moves(blockers, iking).mask(board.allies());
    candidates |=
        magic::rooks::get_avail_moves(blockers, iking).mask(board.allies());

    bitboard discovered = blockers.exclude(candidates);

    // get the sliders that were discovered

    pinners = magic::bishops::get_avail_moves(discovered, iking)
                  .mask(board.enemies())
                  .mask(board.bishops | board.queens);

    pinners |= magic::rooks::get_avail_moves(discovered, iking)
                   .mask(board.enemies())
                   .mask(board.rooks | board.queens);

    struct {
        bitboard before;
        bitboard after;
    } side;

    side.before = king - 1;
    side.after = ~side.before;

    union {
        struct {
            bitboard_t vertical;
            bitboard_t horizontal;
            bitboard_t diagonal;
            bitboard_t rdiagonal;
        };
        bitboard all[4] = {0};
    } sliders;

    sliders.diagonal = bitboard::masks::diagonal_at(iking);
    sliders.rdiagonal = bitboard::masks::rev_diagonal_at(iking);
    sliders.vertical = bitboard::masks::file(iking.column());
    sliders.horizontal = bitboard::masks::rank(iking.row());

    for (const bitboard& slider : sliders.all) {
        bitboard curr_mask;

        curr_mask = slider.mask(side.before);

        pinned |= candidates
                      .mask(curr_mask)  //
                      .mask(-(!!pinners.mask(curr_mask)));

        curr_mask = slider.mask(side.after);

        pinned |= candidates
                      .mask(curr_mask)  //
                      .mask(-(!!pinners.mask(curr_mask)));
    }

    bitboard partial_pin = 0;

    partial = pinned.mask(board.queens);

    partial |=
        pinned.mask(board.rooks).mask(sliders.vertical | sliders.horizontal);

    partial |=
        pinned.mask(board.bishops).mask(sliders.diagonal | sliders.rdiagonal);

    absolute = pinned.exclude(partial);
    partial = partial;
}

}  // namespace core
