#include <bit>
#include <cstdint>

namespace Game {

    typedef uint64_t bitboard_t;
    typedef uint8_t square_t;

    template <typename type> class LiteralWrapper {
      protected:
        type value_ = 0;

      public:
        constexpr LiteralWrapper<type>(type value) : value_(value) {}

        // Overload operators to forward to the wrapped type
        constexpr operator type &() { return value_; }
        constexpr operator const type &() const { return value_; }

        // Forward other operations to the underlying type as needed
        constexpr type *operator->() { return &value_; }
        constexpr const type *operator->() const { return &value_; }
    };

    struct square : public LiteralWrapper<square_t> {
        public:
            constexpr square(square_t value = 0) : LiteralWrapper<square_t>(value) {}
    };

    struct bitboard : public LiteralWrapper<bitboard_t> {
      public:
        constexpr bitboard(bitboard_t value = 0)
            : LiteralWrapper<bitboard_t>(value) {}

        static inline constexpr bitboard bit_at(square index) {
            return 1ul << index;
        }

        inline constexpr bool is_set_at(square index) {
            return (value_ >> index) & 1ul;
        }

        inline constexpr bitboard last_bit() { return value_ & 1; }

        inline constexpr uint8_t popcount() { return std::popcount(value_); }
        inline constexpr uint8_t rzeros() { return std::countr_zero(value_); }

        inline constexpr bitboard flip_vertically() {
            auto value = value_;

            const uint64_t k1 = 0x00FF00FF00FF00FFul;
            const uint64_t k2 = 0x0000FFFF0000FFFFul;
            value = ((value >> 8) & k1) | ((value & k1) << 8);
            value = ((value >> 16) & k2) | ((value & k2) << 16);
            value = (value >> 32) | (value << 32);
            return value;
        }

        inline constexpr bitboard flip_horizontally() {
            auto value = value_;

            const uint64_t k1 = 0x5555555555555555ul;
            const uint64_t k2 = 0x3333333333333333ul;
            const uint64_t k4 = 0x0f0f0f0f0f0f0f0ful;
            value = ((value >> 1) & k1) + 2 * (value & k1);
            value = ((value >> 2) & k2) + 4 * (value & k2);
            value = ((value >> 4) & k4) + 16 * (value & k4);
            return value;
        }
    };
} // namespace Game
