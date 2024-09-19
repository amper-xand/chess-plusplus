#include "magic.hpp"

#include "../../../utils/masks.hpp"
#include "../../../utils/utils.hpp"

#include <bit>
#include <cstdint>
#include <stdexcept>

using namespace Utils;

namespace Game::Generators::Magic {
    struct MagicEntry {
        bitboard mask;
        uint64_t magic;
        uint8_t bits;
    };

    uint16_t get_magic_index(struct MagicEntry magic, bitboard blockers) {
        blockers = blockers & magic.mask;
        const uint64_t hash = blockers * magic.magic;

        const uint16_t index = hash >> (64 - magic.bits);

        return index;
    }
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
    bitboard get_moves(MagicEntry entry, bitboard moves[], bitboard blockers,
                       Game::square index) {
        return moves[get_magic_index(entry, blockers)];
    }

} // namespace Game::Generators::Magic

namespace Game::Generators::Magic::Rooks {
    constexpr uint8_t bits = 12;

    MagicEntry entries[64];

    bitboard avail_moves[64][Utils::bit_at(bits)];

#include "magicsr.data"

    inline bitboard get_relevant_blockers(square index) {
        return (Masks::horizontal_rel_blo_mask << Utils::start_of_row(index)) ^
               (Masks::vertical_rel_blo_mask << Utils::column(index));
    }

    inline bitboard get_slider(square index) {
        return (Masks::horizontal_moves_mask << Utils::start_of_row(index)) ^
               (Masks::vertical_moves_mask << Utils::column(index));
    }

    bitboard get_avail_moves(bitboard blockers, square index) {
        return get_moves(entries[index], avail_moves[index],
                         blockers & get_relevant_blockers(index), index);
    }

    bitboard get_slider_moves(bitboard blockers, square index) {
        const bitboard slider = get_slider(index);

        // Masks the blockers with a north ray
        const bitboard north_mask = Masks::make_n_mask(index);
        bitboard n_ray = blockers & (slider & north_mask);
        n_ray &= ~n_ray + 1;
        // Turn it into a ray from the blocker
        n_ray |= n_ray - 1;
        n_ray &= slider & north_mask;

        const bitboard south_mask = Masks::make_s_mask(index);
        bitboard s_ray = blockers & (slider & south_mask);
        s_ray = Utils::flip_vertically(s_ray);
        s_ray &= ~s_ray + 1;
        // Turn it into a ray from the blocker
        s_ray |= s_ray - 1;
        s_ray = Utils::flip_vertically(s_ray);
        s_ray &= slider & south_mask;

        const bitboard west_mask = Masks::make_w_mask(index);
        bitboard w_ray = blockers & (slider & west_mask);
        // Turn it into a ray from the blocker
        w_ray &= ~w_ray + 1;
        w_ray |= w_ray - 1;
        w_ray &= slider & west_mask;

        const bitboard east_mask = Masks::make_e_mask(index);
        bitboard e_ray = blockers & (slider & east_mask);
        e_ray = Utils::flip_horizontally(e_ray);
        e_ray &= ~e_ray + 1;
        e_ray |= e_ray - 1;
        e_ray = Utils::flip_horizontally(e_ray);
        e_ray &= slider & east_mask;

        return n_ray | s_ray | w_ray | e_ray;
    }

} // namespace Game::Generators::Magic::Rooks

namespace Game::Generators::Magic::Bishops {
    constexpr uint8_t bits = 9;

    MagicEntry entries[64];

    bitboard avail_moves[64][Utils::bit_at(bits)];

#include "magicsb.data"

    inline bitboard get_slider(square index) {
        return Masks::get_diagonal_at(index) ^
               Masks::get_rev_diagonal_at(index);
    }

    inline bitboard get_relevant_blockers(square index) {
        return get_slider(index) & ~Masks::border_mask;
    }

    bitboard get_avail_moves(bitboard blockers, square index) {
        return get_moves(entries[index], avail_moves[index],
                         blockers & get_relevant_blockers(index), index);
    }

    bitboard get_slider_moves(bitboard blockers, square index) {
        const bitboard slider = get_slider(index);

        const bitboard n_mask = Masks::make_n_mask(index);
        const bitboard s_mask = Masks::make_s_mask(index);
        const bitboard w_mask = Masks::make_w_mask(index);
        const bitboard e_mask = Masks::make_e_mask(index);

        const bitboard nw_mask = n_mask & w_mask;
        bitboard nw_ray = blockers & (slider & nw_mask);
        nw_ray &= ~nw_ray + 1;
        nw_ray |= nw_ray - 1;
        nw_ray &= slider & nw_mask;

        const bitboard ne_mask = n_mask & e_mask;
        bitboard ne_ray = blockers & (slider & ne_mask);
        ne_ray &= ~ne_ray + 1;
        ne_ray |= ne_ray - 1;
        ne_ray &= slider & ne_mask;

        const bitboard sw_mask = s_mask & w_mask;
        bitboard sw_ray = blockers & (slider & sw_mask);
        sw_ray = Utils::flip_vertically(sw_ray);
        sw_ray &= ~sw_ray + 1;
        sw_ray |= sw_ray - 1;
        sw_ray = Utils::flip_vertically(sw_ray);
        sw_ray &= slider & sw_mask;

        const bitboard se_mask = s_mask & e_mask;
        bitboard se_ray = blockers & (slider & se_mask);
        se_ray = Utils::flip_vertically(se_ray);
        se_ray &= ~se_ray + 1;
        se_ray |= se_ray - 1;
        se_ray = Utils::flip_vertically(se_ray);
        se_ray &= slider & se_mask;

        return nw_ray | ne_ray | sw_ray | se_ray;
    }
} // namespace Game::Generators::Magic::Bishops

namespace Game::Generators::Magic {

    // mgenerator short of mask generator
    typedef bitboard (*mgenerator)(bitboard blockers, square index);

    void gen_magic_numbers(MagicEntry &entry, bitboard table[], square index,
                           mgenerator generator) {
        const bitboard addition_mask = ~entry.mask;
        const uint32_t table_size = Utils::bit_at(entry.bits);
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
            precalc[blockers_count].moves = generator(curr_blocker_mask, index);

            ++blockers_count;

            if (curr_blocker_mask == entry.mask)
                break;
        }

        // Try to find a magic number
        while (true) {
            do { // Find a suitable magic number
                entry.magic = Random::rnd_composite_low();
            } while (std::popcount(get_magic_index(entry, entry.mask)) < 6);

            // Clear table and try again
            memset((char *)table, 0, table_size * sizeof(*table));

            // Loop through all possible blockers configurations
            for (auto pre : precalc) {
                uint16_t magic_index = get_magic_index(entry, pre.blockers);

                if (table[magic_index] != default_value &&
                    table[magic_index] != pre.moves)
                    break; // Data collisison

                table[magic_index] = pre.moves;

                if (pre.blockers == entry.mask)
                    return; // Finish generation
            }
        }
    }

    void gen_magic_tables() {
        std::printf("Starting rook magic number search\n");

        for (square index = 0; index < 64; ++index) {
            Rooks::entries[index].bits = Rooks::bits;
            Rooks::entries[index].mask = Rooks::get_relevant_blockers(index);

            gen_magic_numbers(Rooks::entries[index], Rooks::avail_moves[index],
                              index, Rooks::get_slider_moves);
        }

        std::printf("Starting bishop magic number search\n");

        for (square index = 0; index < 64; ++index) {
            Bishops::entries[index].bits = Bishops::bits;
            Bishops::entries[index].mask =
                Bishops::get_relevant_blockers(index);

            gen_magic_numbers(Bishops::entries[index],
                              Bishops::avail_moves[index], index,
                              Bishops::get_slider_moves);
        }
    }

    void print_magics() {
        std::printf("uint64_t precalc_rook_magics[] = {\n");
        for (square index = 0; index < 64; ++index) {
            std::printf("0x%lXUL,\n", Rooks::entries[index].magic);
        }

        std::printf("};\n\nuint64_t precalc_bishop_magics[] = {\n");
        for (square index = 0; index < 64; ++index) {
            std::printf("0x%lXUL,\n", Bishops::entries[index].magic);
        }
        std::printf("};");
    }

    void initialize_magic_table(bitboard table[], MagicEntry &entry,
                                square index, mgenerator move_generator) {
        const bitboard addition_mask = ~entry.mask;
        const uint32_t table_size = Utils::bit_at(entry.bits);

        // clang-format off
        for (bitboard curr_blocker_mask = 0;; curr_blocker_mask = (curr_blocker_mask + addition_mask + 1) & entry.mask) { // clang-format on
            const bitboard moves = move_generator(curr_blocker_mask, index);
            const uint16_t magic_index =
                get_magic_index(entry, curr_blocker_mask);

            if (magic_index >= table_size)
                throw std::out_of_range("Magic is out of bounds");

            table[magic_index] = moves;

            if (curr_blocker_mask == entry.mask)
                break;
        }
    }

    void initialize_magic_tables() {
        for (square index = 0; index < 64; ++index) {
            Rooks::entries[index].bits = Rooks::bits;
            Rooks::entries[index].mask = Rooks::get_relevant_blockers(index);
            Rooks::entries[index].magic = Rooks::precalc_magics[index];

            // clang-format off
            Bishops::entries[index].bits = Bishops::bits;
            Bishops::entries[index].mask = Bishops::get_relevant_blockers(index);
            Bishops::entries[index].magic = Bishops::precalc_magics[index];
            // clang-format on

            initialize_magic_table(Rooks::avail_moves[index],
                                   Rooks::entries[index], index,
                                   Rooks::get_slider_moves);

            initialize_magic_table(Bishops::avail_moves[index],
                                   Bishops::entries[index], index,
                                   Bishops::get_slider_moves);
        }
    }

} // namespace Game::Generators::Magic

#ifdef MAGIC_STANDALONE
int main() {
    Game::Generators::Magic::gen_magic_tables();
    Game::Generators::Magic::print_magics();

    return 0;
}
#endif
