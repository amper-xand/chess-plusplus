#include "magic.hpp"

#include <bit>
#include <cstdint>
#include <stdexcept>

using Masks = Game::bitboard::Masks;

namespace Game::Generators::Magic {
    struct MagicEntry {
        bitboard mask;
        uint64_t magic;
        uint8_t bits;

        uint16_t magic_index(bitboard blockers) {
            blockers &= mask;
            const uint64_t hash = blockers * magic;

            const uint16_t index = hash >> (64 - bits);

            return index;
        }
    };

} // namespace Game::Generators::Magic

namespace Random {
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
}; // namespace Random

namespace Game::Generators::Magic {

    template <typename Derived, uint8_t Bits> struct MagicTable {
        static constexpr uint8_t bits = Bits;
        static MagicEntry entries[64];
        static bitboard moves[64]
                             [static_cast<bitboard_t>(bitboard::bit_at(Bits))];

        static inline bitboard relevant_blockers(square index) {
            return Derived::relevant_blockers(index);
        };

        static inline bitboard slider(square) { return Derived::slider(index); }

        static bitboard gen_moves(bitboard blockers, square index) {
            return Derived::gen_moves(blockers, index);
        }

        static bitboard get_moves(bitboard blockers, square index) {
            auto rel_blockers = relevant_blockers(index).mask(blockers);
            auto magic_index = entries[index].magic_index(rel_blockers);

            return moves[index][magic_index];
        }
    };

    // Define the static members outside the class template
    template <typename Derived, uint8_t Bits>
    MagicEntry MagicTable<Derived, Bits>::entries[64];

    template <typename Derived, uint8_t Bits>
    bitboard MagicTable<Derived, Bits>::moves[64][static_cast<bitboard_t>(
        bitboard::bit_at(Bits))];

    struct Rookst : public MagicTable<Rookst, 12> {

#include "magicsr.data"

        static inline bitboard relevant_blockers(square index) {
            return (Masks::rel_blockers_horizontal << index.start_of_row()) ^
                   (Masks::rel_blockers_vertical << index.column());
        }

        static inline bitboard slider(square index) {
            return (Masks::horizontal << index.start_of_row()) ^
                   (Masks::vertical << index.column());
        }

        static bitboard gen_moves(bitboard blockers, square index) {
            const bitboard slider = Rookst::slider(index);

            // Masks the blockers with a north ray
            const bitboard north_mask = Masks::make_n_mask(index);
            bitboard n_ray = blockers.mask(slider & north_mask);
            n_ray &= ~n_ray + 1;
            // Turn it into a ray from the blocker
            n_ray |= n_ray - 1;
            n_ray &= slider & north_mask;

            const bitboard south_mask = Masks::make_s_mask(index);
            bitboard s_ray = blockers.mask(slider & south_mask);
            s_ray = s_ray.flip_vertically();
            s_ray &= ~s_ray + 1;
            // Turn it into a ray from the blocker
            s_ray |= s_ray - 1;
            s_ray = s_ray.flip_vertically();
            s_ray &= slider & south_mask;

            const bitboard west_mask = Masks::make_w_mask(index);
            bitboard w_ray = blockers.mask(slider & west_mask);
            // Turn it into a ray from the blocker
            w_ray &= ~w_ray + 1;
            w_ray |= w_ray - 1;
            w_ray &= slider & west_mask;

            const bitboard east_mask = Masks::make_e_mask(index);
            bitboard e_ray = blockers.mask(slider & east_mask);
            e_ray = e_ray.flip_horizontally();
            e_ray &= ~e_ray + 1;
            e_ray |= e_ray - 1;
            e_ray = e_ray.flip_horizontally();
            e_ray &= slider & east_mask;

            return n_ray | s_ray | w_ray | e_ray;
        }
    };

    struct Bishopst : public MagicTable<Bishopst, 9> {

#include "magicsb.data"

        static inline bitboard relevant_blockers(square index) {
            return slider(index).mask(~Masks::border);
        }

        static inline bitboard slider(square index) {
            return Masks::get_diagonal_at(index) ^
                   Masks::get_rev_diagonal_at(index);
        }

        static bitboard gen_moves(bitboard blockers, square index) {
            const bitboard slider = Bishopst::slider(index);

            const bitboard n_mask = Masks::make_n_mask(index);
            const bitboard s_mask = Masks::make_s_mask(index);
            const bitboard w_mask = Masks::make_w_mask(index);
            const bitboard e_mask = Masks::make_e_mask(index);

            const bitboard nw_mask = n_mask & w_mask;
            bitboard nw_ray = blockers.mask(slider & nw_mask);
            nw_ray &= ~nw_ray + 1;
            nw_ray |= nw_ray - 1;
            nw_ray &= slider & nw_mask;

            const bitboard ne_mask = n_mask & e_mask;
            bitboard ne_ray = blockers.mask(slider & ne_mask);
            ne_ray &= ~ne_ray + 1;
            ne_ray |= ne_ray - 1;
            ne_ray &= slider & ne_mask;

            const bitboard sw_mask = s_mask & w_mask;
            bitboard sw_ray = blockers.mask(slider & sw_mask);
            sw_ray = sw_ray.flip_vertically();
            sw_ray &= ~sw_ray + 1;
            sw_ray |= sw_ray - 1;
            sw_ray = sw_ray.flip_vertically();
            sw_ray &= slider & sw_mask;

            const bitboard se_mask = s_mask & e_mask;
            bitboard se_ray = blockers.mask(slider & se_mask);
            se_ray = se_ray.flip_vertically();
            se_ray &= ~se_ray + 1;
            se_ray |= se_ray - 1;
            se_ray = se_ray.flip_vertically();
            se_ray &= slider & se_mask;

            return nw_ray | ne_ray | sw_ray | se_ray;
        }
    };

} // namespace Game::Generators::Magic

namespace Game::Generators::Magic::Rooks {

    bitboard get_avail_moves(bitboard blockers, square index) {
        return Rookst::get_moves(blockers, index);
    }

} // namespace Game::Generators::Magic::Rooks

namespace Game::Generators::Magic::Bishops {

    bitboard get_avail_moves(bitboard blockers, square index) {
        return Bishopst::get_moves(blockers, index);
    }

} // namespace Game::Generators::Magic::Bishops

namespace Game::Generators::Magic {

    template <typename Table> void search_magic(square index) {

        auto& entry = Table::entries[index];
        entry.bits = Table::bits;
        entry.mask = Table::relevant_blockers(index);

        auto& table = Table::moves[index];

        const bitboard addition_mask = ~entry.mask;
        const uint32_t table_size = 1 << entry.bits;
        constexpr uint64_t default_value = 0;

        // Precalculate the moves bitboards for every blocker
        const uint16_t max_blockers_com = table_size;

        struct Precalc {
            bitboard blockers, moves;
        };

        Precalc precalc[max_blockers_com];

        uint16_t blockers_count = 0;

        // clang-format off
        for (bitboard curr_blocker_mask = 0;;
             curr_blocker_mask = (curr_blocker_mask + addition_mask + 1) & entry.mask) { // clang-format on

            if (blockers_count >= max_blockers_com) {
                throw std::out_of_range("Blockers exceeded allocated area");
            }

            precalc[blockers_count].blockers = curr_blocker_mask;
            precalc[blockers_count].moves =
                Table::gen_moves(curr_blocker_mask, index);

            ++blockers_count;

            if (curr_blocker_mask == entry.mask)
                break;
        }

        // Try to find a magic number
        while (true) {
            do { // Find a suitable magic number
                entry.magic = Random::rnd_composite_low();
            } while (std::popcount(entry.magic_index(entry.mask)) < 6);

            // Clear table and try again
            memset((char *)table, 0, table_size * sizeof(*table));

            // Loop through all possible blockers configurations
            for (auto pre : precalc) {
                uint16_t magic_index = entry.magic_index(pre.blockers);

                if (table[magic_index] != default_value &&
                    table[magic_index] != pre.moves)
                    break; // Data collisison

                table[magic_index] = pre.moves;

                if (pre.blockers == entry.mask)
                    return; // Finish generation
            }
        }
    }

    void search_magics() {
        std::printf("Starting rook magic number search\n");

        for (square index = 0; index < 64; ++index) {
            search_magic<Rookst>(index);
        }

        std::printf("Starting bishop magic number search\n");

        for (square index = 0; index < 64; ++index) {
            search_magic<Bishopst>(index);
        }
    }

    void print_magics() {
        std::printf("static constexpr uint64_t precalc_rook_magics[64]{\n");
        for (square index = 0; index < 64; ++index) {
            std::printf("0x%lXUL,\n", Rookst::entries[index].magic);
        }

        std::printf(
            "};\n\nstatic constexpr uint64_t precalc_bishop_magics[64]{\n");
        for (square index = 0; index < 64; ++index) {
            std::printf("0x%lXUL,\n", Bishopst::entries[index].magic);
        }
        std::printf("};");
    }

    template <typename Table> void initialize_magic_table(square index) {
        auto& entry = Table::entries[index];

        entry.bits = Table::bits;
        entry.mask = Table::relevant_blockers(index);
        entry.magic = Table::precalc_magics[index];

        const bitboard addition_mask = ~entry.mask;
        const uint32_t table_size = 1 << entry.bits;

        // clang-format off
        for (bitboard curr_blocker_mask = 0;; curr_blocker_mask = (curr_blocker_mask + addition_mask + 1) & entry.mask) { // clang-format on
            const bitboard moves = Table::gen_moves(curr_blocker_mask, index);
            const uint16_t magic_index = entry.magic_index(curr_blocker_mask);

            if (magic_index >= table_size)
                throw std::out_of_range("Magic is out of bounds");

            Table::moves[index][magic_index] = moves;

            if (curr_blocker_mask == entry.mask)
                break;
        }
    }

    void initialize_magic_tables() {
        for (square index = 0; index < 64; ++index) {
            initialize_magic_table<Rookst>(index);
            initialize_magic_table<Bishopst>(index);
        }
    }

} // namespace Game::Generators::Magic

#ifdef MAGIC_STANDALONE
int main() {
    Game::Generators::Magic::search_magics();
    Game::Generators::Magic::print_magics();

    return 0;
}
#endif
