#include "core/magic.hpp"

#include <array>
#include <cassert>
#include <cstring>
#include <print>

namespace core::generation::magic {

namespace rnd {
uint64_t seed = 3982143279794049853;

inline uint64_t random() {
    // XOR shift algorithm
    seed ^= seed << 13;
    seed ^= seed >> 17;
    seed ^= seed << 5;

    return seed;
}

inline uint64_t rnd_composite() {
    uint64_t u1, u2, u3, u4;

    u1 = random() & 0xFFFF;
    u2 = random() & 0xFFFF;
    u3 = random() & 0xFFFF;
    u4 = random() & 0xFFFF;

    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

inline uint64_t random_low() {
    return random() & random() & random() & random();
}

inline uint64_t rnd_composite_low() {
    return rnd_composite() & rnd_composite() & rnd_composite();
}
}  // namespace rnd

struct MagicEntry {
    bitboard mask;
    uint64_t magic;
    uint8_t bits;

    constexpr uint16_t magic_index(bitboard blockers) {
        blockers &= mask;
        const uint64_t hash = blockers * magic;

        const uint16_t index = hash >> (64 - bits);

        return index;
    }
};

template <typename Derived, uint8_t Bits>
struct MagicTable {
    static constexpr uint8_t bits = Bits;
    static MagicEntry entries[64];

    static constexpr bitboard table_size = bitboard::masks::at(Bits);
    static std::array<bitboard, table_size> moves[64];

    static const std::array<uint64_t, 64> precalc_magics;

    static inline bitboard relevant_blockers(square index) {
        return Derived::relevant_blockers(index);
    };
    static inline bitboard slider(square index) {
        return Derived::slider(index);
    }

    static bitboard gen_moves(bitboard blockers, square index) {
        return Derived::gen_moves(blockers, index);
    }

    static bitboard get_moves(bitboard blockers, square index) {
        auto rel_blockers = relevant_blockers(index).mask(blockers);
        auto magic_index = entries[index].magic_index(rel_blockers);

        return moves[index][magic_index];
    }

    static MagicEntry search_magic(square index) {
        // Fill base entry data
        MagicEntry entry;
        entry.bits = MagicTable::bits;
        entry.mask = MagicTable::relevant_blockers(index);

        std::array<bitboard, MagicTable::table_size> table = {0};

        // Precalculate the moves bitboards for every blocker

        struct Precalc {
            bitboard blockers, moves;
        };

        const uint16_t max_blockers_com = table.size();
        std::array<Precalc, max_blockers_com> precalc;

        bitboard curr_blocker_mask = entry.mask;
        constexpr bitboard last_blo_com = 0;

        for (uint16_t blockers_count = 0;;
            curr_blocker_mask = (curr_blocker_mask - 1) & entry.mask) {
            // fill the precalculated entry
            precalc[blockers_count].blockers = curr_blocker_mask;
            precalc[blockers_count].moves =
                MagicTable::gen_moves(curr_blocker_mask, index);

            ++blockers_count;

            // check if we just filled
            // the last combination of blockers
            if (curr_blocker_mask == last_blo_com) break;
        }

        // Try to find a magic number

        constexpr uint64_t cleared_value = 0;

        while (true) {
            do {  // Find a suitable magic number
                entry.magic = rnd::rnd_composite_low();
            } while (std::popcount(entry.magic_index(entry.mask)) < 6);

            // Clear table and try again
            std::memset((char*)table.data(), cleared_value,
                table_size * sizeof(*table.data()));

            // Loop through all possible blockers configurations
            for (Precalc pre : precalc) {
                uint16_t magic_index = entry.magic_index(pre.blockers);

                if (table[magic_index] != cleared_value &&
                    table[magic_index] != pre.moves)
                    break;  // Data collisison

                table[magic_index] = pre.moves;

                if (pre.blockers == last_blo_com)
                    return entry;  // Finish generation
            }
        }
    }

    static std::array<uint64_t, 64> precalculate_magic() {
        std::array<uint64_t, 64> precalc_magics = {0};

        for (square index = 0; index < 64; ++index)
            precalc_magics[index] = MagicTable::search_magic(index).magic;

        return precalc_magics;
    }

#ifndef MAGIC_STANDALONE
    static void initialize() {
        for (square index = 0; index < 64; ++index) {
            MagicEntry& entry = MagicTable::entries[index];

            entry.bits = MagicTable::bits;
            entry.magic = MagicTable::precalc_magics[index];
            entry.mask = MagicTable::relevant_blockers(index);

            constexpr bitboard last_blo_com = 0;
            constexpr uint64_t cleared_value = 0;

            for (bitboard curr_blocker_mask = entry.mask;;
                curr_blocker_mask = (curr_blocker_mask - 1) & entry.mask) {
                uint16_t magic_index = entry.magic_index(curr_blocker_mask);

                bitboard moves =
                    MagicTable::gen_moves(curr_blocker_mask, index);

                assert(MagicTable::moves[index][magic_index] == cleared_value ||
                    MagicTable::moves[index][magic_index] == moves);

                MagicTable::moves[index][magic_index] = moves;

                // check if we just filled
                // the last combination of blockers
                if (curr_blocker_mask == last_blo_com) break;
            }
        }
    }

   private:
    inline static struct Initializer {
        Initializer() { MagicTable::initialize(); }
    } initializer;
#endif
};

// Define the static members outside the class template
template <typename Derived, uint8_t Bits>
MagicEntry MagicTable<Derived, Bits>::entries[64];

template <typename Derived, uint8_t Bits>
std::array<bitboard, MagicTable<Derived, Bits>::table_size>
    MagicTable<Derived, Bits>::moves[64] = {0};

using masks = core::bitboard::masks;

struct Rookst : public MagicTable<Rookst, 12> {
    static inline bitboard relevant_blockers(square index) {
        static constexpr bitboard rel_blockers_vertical =
            masks::vertical & ~0x0100000000000001;
        static constexpr bitboard rel_blockers_horizontal =
            masks::horizontal & ~0x81;

        return (rel_blockers_vertical << index.column()) ^
            (rel_blockers_horizontal << (index.row() * 8));
    }

    static inline bitboard slider(square index) {
        return (masks::horizontal << (index.row() * 8)) ^
            (masks::vertical << index.column());
    }

    static bitboard gen_moves(bitboard blockers, square index) {
        const bitboard slider = Rookst::slider(index);
        blockers = blockers.mask(slider);

        const bitboard rook = masks::at(index);

        const bitboard vertical = masks::file(index.column());

        // get the blockers that are over the rook
        bitboard n_ray = blockers.mask(vertical).exclude(rook - 1);
        // set the bits between the first blocker and the rook, without the rook
        n_ray = masks::interval(rook, n_ray.LSB()).mask(vertical) ^ rook;

        // get the blockers that are under the rook
        bitboard s_ray =
            // {masks::rank(0)} extra blocker to keep the
            // interval on the right side
            blockers.join(masks::rank(0))
                .mask(vertical)
                .mask(rook | (rook - 1));
        // set the bits between the first blocker and the rook
        s_ray = masks::interval(rook, s_ray.MSB()).mask(vertical) ^ rook;

        const bitboard horizontal = masks::rank(index.row());

        // get the blockers left to the rook
        bitboard w_ray = blockers.mask(horizontal).exclude(rook - 1);
        // set the bits between the first blocker and the rook, without the rook
        w_ray = masks::interval(rook, w_ray.LSB()).mask(horizontal) ^ rook;

        // get the blockers left to the rook
        bitboard e_ray =
            // {masks::file(0)} extra blocker to keep the
            // interval on the right side
            blockers.join(masks::file(0))
                .mask(horizontal)
                .mask(rook | (rook - 1));
        // set the bits between the first blocker and the rook, without the rook
        e_ray = masks::interval(rook, e_ray.MSB()).mask(horizontal) ^ rook;

        return n_ray | s_ray | w_ray | e_ray;
    }
};

// force initialization
template struct MagicTable<Rookst, 12>;

bitboard rooks::get_avail_moves(bitboard blockers, square index) {
    return Rookst::get_moves(blockers, index);
}

bitboard rooks::get_slider(square index) { return Rookst::slider(index); }

template <typename Rookst, uint8_t Bits>
// Generated using maggen binary
const std::array<uint64_t, 64> MagicTable<Rookst, Bits>::precalc_magics = {
    0x2080028220400012UL,
    0xD40009000600048UL,
    0xA280081000A00006UL,
    0x280049000020800UL,
    0x800608000400C1UL,
    0x1080040051048600UL,
    0x4600040228004183UL,
    0x200008600C12304UL,
    0x8000201A4004UL,
    0x8101001080040UL,
    0x2000E00064004020UL,
    0x404002081100UL,
    0x6002002052008804UL,
    0x808004084222408UL,
    0x12084100404080UL,
    0x242000841240102UL,
    0x248000104000UL,
    0x100801040080440CUL,
    0x24080C2002048A02UL,
    0x401C20400E080040UL,
    0x880804880040100UL,
    0x283D010122000400UL,
    0x200004002800100UL,
    0x3002450000403082UL,
    0x2080201080084000UL,
    0x4204000AA0014110UL,
    0x88004090045080UL,
    0x4418440040524UL,
    0x1010010022080C0UL,
    0x20109010042C0020UL,
    0x2802802028100040UL,
    0x804040200008033UL,
    0x82164000A1800015UL,
    0x410106002800881UL,
    0x8021060200A1000UL,
    0x294218D001000501UL,
    0x4100200402201100UL,
    0x1801C0200210080UL,
    0x20084200004000A1UL,
    0x8060002041800100UL,
    0x2040400820108001UL,
    0x48100308088400UL,
    0x50094800210800C0UL,
    0x101804800C10284UL,
    0x1800080200010300UL,
    0xA500200104A8004UL,
    0x1020210008201UL,
    0x81825411820009UL,
    0xE0284011800100UL,
    0x4888800100818UL,
    0xE00048A002100008UL,
    0x810410000280UL,
    0x2008800458026100UL,
    0x204008401041020UL,
    0x800024A002110440UL,
    0x842344038200UL,
    0x100010820900C162UL,
    0x4102100AC0018021UL,
    0xA88020104086UL,
    0xA111820400402UL,
    0x2021004080003UL,
    0x20080228840001UL,
    0x810440B040624804UL,
    0x80040B0348840022UL,
};

}  // namespace core::generation::magic

#ifdef MAGIC_STANDALONE

int main() {
    std::printf("Starting rook magic number search\n");
    auto magics = core::generation::magic::Rookst::precalculate_magic();

    std::println("static constexpr uint64_t precalc_rook_magics[64]{{");
    for (auto magic : magics) {
        std::println("0x{:X}UL,", magic);
    }
    std::println("}};");
}

#endif
