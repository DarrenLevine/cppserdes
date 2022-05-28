#include "test_utilities.h"

template <typename test_type>
void nominal_testing()
{
    static constexpr size_t bits = sizeof(test_type) * 8u;

#ifdef __SIZEOF_INT128__
    uint64_t test_pattern1 = 6345174947073056788ULL;
    __uint128_t test_pattern = test_pattern1;
    test_pattern |= (static_cast<__uint128_t>(~test_pattern1) << 64u);
#else
    uint64_t test_pattern = 6345174947073056788ULL;
#endif

    const test_type ffff_val = ~test_type();
    const test_type check_pattern = static_cast<test_type>(test_pattern & ffff_val);
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, bits, 0u, {check_pattern, ffff_val, ffff_val, ffff_val});
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, bits, bits, {ffff_val, check_pattern, ffff_val, ffff_val});
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, bits, bits * 3u, {ffff_val, ffff_val, ffff_val, check_pattern});

#ifdef __SIZEOF_INT128__
    const __uint128_t inv_test_pattern = ~test_pattern;
#else
    const uint64_t inv_test_pattern = ~test_pattern;
#endif
    // THese fail if #undef __SIZEOF_INT128__
    const test_type inv_check_pattern = ~check_pattern;
    test_type z_val = test_type();
    test_bitcpy_insert<test_type>(__LINE__, inv_test_pattern, bits, 0u, {inv_check_pattern, z_val, z_val, z_val}, z_val);
    test_bitcpy_insert<test_type>(__LINE__, inv_test_pattern, bits, bits, {z_val, inv_check_pattern, z_val, z_val}, z_val);
    test_bitcpy_insert<test_type>(__LINE__, inv_test_pattern, bits, bits * 3u, {z_val, z_val, z_val, inv_check_pattern}, z_val);
}

template <typename test_type>
void underflow_under_array_boundary_testing()
{
    // using only the least significant 4 bits
#ifdef __SIZEOF_INT128__
    __uint128_t test_pattern = 6004798787331442ULL;
#else
    uint64_t test_pattern = 6004798787331442ULL;
#endif
    const test_type ffff_val = ~test_type();
    static constexpr size_t bits = sizeof(test_type) * 8u;
    auto get_match_pattern = [=](size_t x) -> test_type
    {
        size_t right_shift = 4u + (x % bits);
        test_type match_pattern = 0u;
        size_t left_shift = bits - right_shift + 4u;
        if (left_shift < bits)
            match_pattern |= ffff_val << left_shift;
        if (right_shift < bits)
            match_pattern |= ffff_val >> right_shift;
        match_pattern |= (static_cast<test_type>(test_pattern & 15U) << (bits - right_shift));
        return match_pattern;
    };
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, 4, 0, {get_match_pattern(0u), ffff_val, ffff_val, ffff_val});
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, 4, bits - 4, {get_match_pattern(bits - 4), ffff_val, ffff_val, ffff_val});
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, 4, bits, {ffff_val, get_match_pattern(bits), ffff_val, ffff_val});
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, 4, bits + 1, {ffff_val, get_match_pattern(bits + 1), ffff_val, ffff_val});
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, 4, bits + 3, {ffff_val, get_match_pattern(bits + 3), ffff_val, ffff_val});
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, 4, bits * 3, {ffff_val, ffff_val, ffff_val, get_match_pattern(bits * 3)});
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, 4, bits * 3 + 3, {ffff_val, ffff_val, ffff_val, get_match_pattern(bits * 3 + 3)});
    test_bitcpy_insert<test_type>(__LINE__, test_pattern, 4, bits * 4 - 4, {ffff_val, ffff_val, ffff_val, get_match_pattern(bits * 4 - 4)});
}

template <typename test_type>
void underflow_over_array_boundary_testing()
{
#ifdef __SIZEOF_INT128__
    __uint128_t test_pattern = 14U;
#else
    uint64_t test_pattern = 14U;
#endif
    const size_t bit_size = 5;
    test_type ff_val = ~test_type();
    for (size_t shift_val = 1; shift_val < 6; shift_val++)
    {
        test_type l_pattern = (ff_val << shift_val) | (static_cast<test_type>(test_pattern & 31U) >> (bit_size - shift_val));
        test_type r_pattern = (ff_val >> (bit_size - shift_val));
        if (shift_val - bit_size > 0) // on ARM platforms left bitshifts on 128 bit types of 128 bits fail, so this check is needed for that case, to calculate the correct expected pattern
            r_pattern |= (static_cast<test_type>(test_pattern & 31U) << (sizeof(test_type) * 8 + shift_val - bit_size));
        test_bitcpy_insert<test_type>(__LINE__, test_pattern, bit_size, sizeof(test_type) * 8 * 2 - shift_val, {ff_val, l_pattern, r_pattern, ff_val});
    }
}
static void overflow_over_array_boundary_testing()
{
    BITCPY_INT128_CONDITIONAL_DEFINE_C(
        {
            using test_type = __uint128_t;
            size_t bit_size = sizeof(test_type) * 8;
            test_type ffff_val = ~test_type();
            __uint128_t test_pattern = 0xABCD012345678901llu;
            test_pattern = (test_pattern << 64) | 0x2233445566778890llu;
            test_bitcpy_insert<test_type, false>(__LINE__, test_pattern, bit_size, 4, {ffff_val << 124 | test_pattern >> 4, ffff_val >> 4, ffff_val, ffff_val});
            test_bitcpy_insert<test_type, false>(__LINE__, test_pattern, bit_size, 128, {ffff_val, test_pattern, ffff_val, ffff_val});
        });

#ifdef __SIZEOF_INT128__
    __uint128_t test_pattern = 0xABCD012345678901llu;
    test_pattern = (test_pattern << 64) | 0x2233445566778890llu;
#else
    uint64_t test_pattern = 0xABCD012345678901llu;
#endif

    size_t bit_size = sizeof(test_pattern) * 8;
#ifdef __SIZEOF_INT128__
    test_bitcpy_insert<uint8_t, false>(__LINE__, test_pattern, bit_size, 4u, {0xFA_u8, 0xBC_u8, 0xD0_u8, 0x12_u8, 0x34_u8, 0x56_u8, 0x78_u8, 0x90_u8, 0x12_u8, 0x23_u8, 0x34_u8, 0x45_u8, 0x56_u8, 0x67_u8, 0x78_u8, 0x89_u8, 0x0F_u8, 0xFF_u8});
    test_bitcpy_insert<uint16_t, false>(__LINE__, test_pattern, bit_size, 4u, {0xFABC_u16, 0xD012_u16, 0x3456_u16, 0x7890_u16, 0x1223_u16, 0x3445_u16, 0x5667_u16, 0x7889_u16, 0x0FFF_u16});
    test_bitcpy_insert<uint32_t, false>(__LINE__, test_pattern, bit_size, 4u, {0xFABCD012_u32, 0x34567890_u32, 0x12233445_u32, 0x56677889_u32, 0x0FFFFFFF_u32});
    test_bitcpy_insert<uint64_t, false>(__LINE__, test_pattern, bit_size, 4u, {0xFABCD01234567890_u64, 0x1223344556677889_u64, 0x0FFFFFFFFFFFFFFF_u64});
    test_bitcpy_insert<__uint128_t, false>(
        __LINE__,
        test_pattern, bit_size, 4u,
        {static_cast<__uint128_t>(0xFABCD01234567890) << 64 | static_cast<__uint128_t>(0x1223344556677889),
         static_cast<__uint128_t>(0x0FFFFFFFFFFFFFFF) << 64 | static_cast<__uint128_t>(0xFFFFFFFFFFFFFFFF)});
#else
    test_bitcpy_insert<uint8_t, false>(__LINE__, test_pattern, bit_size, 4u, {0xFA_u8, 0xBC_u8, 0xD0_u8, 0x12_u8, 0x34_u8, 0x56_u8, 0x78_u8, 0x90_u8, 0x1F_u8, 0xFF_u8});
    test_bitcpy_insert<uint16_t, false>(__LINE__, test_pattern, bit_size, 4u, {0xFABC_u16, 0xD012_u16, 0x3456_u16, 0x7890_u16, 0x1FFF_u16, 0xFFFF_u16});
    test_bitcpy_insert<uint32_t, false>(__LINE__, test_pattern, bit_size, 4u, {0xFABCD012_u32, 0x34567890_u32, 0x1FFFFFFF_u32, 0xFFFFFFFF_u32});
    test_bitcpy_insert<uint64_t, false>(__LINE__, test_pattern, bit_size, 4u, {0xFABCD01234567890_u64, 0x1FFFFFFFFFFFFFFF_u64, 0xFFFFFFFFFFFFFFFF_u64});
#endif
}

static void test_to_array_booleans()
{
    test_bitcpy_insert<uint8_t>(__LINE__, true, 4_u8, 1_u8, {8_u8, 0_u8, 0_u8}, 0_u8);
    test_bitcpy_insert<uint8_t>(__LINE__, false, 4_u8, 1_u8, {135_u8, 0xff_u8, 0xff_u8});
    test_bitcpy_insert<uint8_t>(__LINE__, true, 4_u8, 6_u8, {0_u8, 64_u8, 0_u8}, 0_u8);
    test_bitcpy_insert<uint8_t>(__LINE__, false, 4_u8, 6_u8, {252_u8, 63_u8, 0xff_u8});
}

static void test_to_array_enums()
{
    enum class temp_e : uint32_t
    {
        QUICK,
        BROWN,
        FOX = 0x1234ABCD
    };

    {
        uint8_t buffer[7] = {};
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), temp_e::FOX);
        test_cmp_arrays<uint8_t, false>(buffer, {0x12_u8, 0x34_u8, 0xAB_u8, 0xCD_u8, 0x00_u8, 0x00_u8, 0x00_u8});
    }
    {
        uint64_t buffer[2] = {};
        serdes::bitcpy(serdes::sized_pointer<uint64_t>(buffer), temp_e::FOX, 52, 16);
        test_cmp_arrays<uint64_t, false>(buffer, {0x0000000000000ABC_u64, 0xD000000000000000_u64});
    }
}

static void test_to_array_floating_point()
{
    {
        uint8_t buffer[9] = {};
        double x = -56789.01234E125;
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), x);
        test_cmp_arrays<uint8_t, false>(buffer, {0xDA_u8, 0xE0_u8, 0x62_u8, 0xA7_u8, 0x65_u8, 0x8C_u8, 0xDA_u8, 0x16_u8, 0x00_u8});
    }
    {
        uint8_t buffer[8] = {};
        float x = 1.1234;
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), x);
        test_cmp_arrays<uint8_t, false>(buffer, {0x3F_u8, 0x8F_u8, 0xCB_u8, 0x92_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8});
    }
    {
        uint8_t buffer[8] = {};
        float x = 1.1234;
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), x, 4, sizeof(x) * 8);
        test_cmp_arrays<uint8_t, false>(buffer, {0x03_u8, 0xF8_u8, 0xFC_u8, 0xB9_u8, 0x20_u8, 0x00_u8, 0x00_u8, 0x00_u8});
    }
    {
        uint8_t buffer[8] = {};
        float x = 1.1234;
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), x, 4, sizeof(x) * 8 - 8);
        test_cmp_arrays<uint8_t, false>(buffer, {0x08_u8, 0xFC_u8, 0xB9_u8, 0x20_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8});
    }
}

static void test_to_array_signed()
{
    {
        uint8_t buffer[8] = {};
        int32_t x = -4;
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), x);
        test_cmp_arrays<uint8_t, false>(buffer, {0xFF_u8, 0xFF_u8, 0xFF_u8, 0xFC_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8});
    }
    {
        uint8_t buffer[8] = {};
        int32_t x = -4;
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), x, 4, 32);
        test_cmp_arrays<uint8_t, false>(buffer, {0x0F_u8, 0xFF_u8, 0xFF_u8, 0xFF_u8, 0xC0_u8, 0x00_u8, 0x00_u8, 0x00_u8});
    }
    {
        uint8_t buffer[8] = {};
        int32_t x = -4;
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), x, 4, 16);
        test_cmp_arrays<uint8_t, false>(buffer, {0x0F_u8, 0xFF_u8, 0xC0_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8});
    }
    {
        int16_t buffer[1] = {};
        typedef uint16_t unsigned_buff_type[1];
        auto &buffer_as_unsigned = *reinterpret_cast<unsigned_buff_type *>(&buffer);

        buffer[0] = 85;
        serdes::detail::extend_sign<int16_t>(buffer[0], 2);
        test_cmp_arrays<uint16_t>(buffer_as_unsigned, {1_u16});

        buffer[0] = 85;
        serdes::detail::extend_sign<int16_t>(buffer[0], 3);
        test_cmp_arrays<uint16_t>(buffer_as_unsigned, {65533_u16});

        buffer[0] = 85;
        serdes::detail::extend_sign(buffer[0], 4);
        test_cmp_arrays<uint16_t>(buffer_as_unsigned, {5_u16});

        buffer[0] = 85;
        serdes::detail::extend_sign(buffer[0], 7);
        test_cmp_arrays<uint16_t>(buffer_as_unsigned, {65493_u16});

        buffer[0] = -2;
        serdes::detail::extend_sign(buffer[0], 16);
        test_cmp_arrays<uint16_t>(buffer_as_unsigned, {uint16_t(-2)});

        buffer[0] = 85;
        serdes::detail::extend_sign(buffer[0], 17);
        test_cmp_arrays<uint16_t>(buffer_as_unsigned, {85_u16});
    }
}

static void test_to_array_large_types()
{
    struct large_class
    {
        uint8_t value[15] = {1_u8, 2_u8, 3_u8, 4_u8, 5_u8, 6_u8, 7_u8, 8_u8, 9_u8, 0xA_u8, 0xB_u8, 0xC_u8, 0xD_u8, 0xE_u8, 0xF_u8};
    };

    large_class x;

    BITCPY_INT128_CONDITIONAL_DEFINE_C(
        {
            __uint128_t f[2] = {};
            serdes::bitcpy(serdes::sized_pointer<__uint128_t>(f), x);
            __uint128_t cmp_pattern = 0x0102030405060708;
            cmp_pattern = (cmp_pattern << 64) | 0x090A0B0C0D0E0F00;
            test_cmp_arrays<__uint128_t, false>(f, {cmp_pattern, cmp_pattern * 0u});
        });
    { // if (bits == total_bits_T_val)
        uint8_t buffer[17] = {};
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), x);
        test_cmp_arrays<uint8_t, false>(buffer, {0x01_u8, 0x02_u8, 0x03_u8, 0x04_u8, 0x05_u8, 0x06_u8, 0x07_u8, 0x08_u8, 0x09_u8, 0x0A_u8, 0x0B_u8, 0x0C_u8, 0x0D_u8, 0x0E_u8, 0x0F_u8, 0x00_u8, 0x00_u8});
    }
    { // if (bits == total_bits_T_val)
        uint16_t buffer[9] = {};
        serdes::bitcpy(serdes::sized_pointer<uint16_t>(buffer), x);
        test_cmp_arrays<uint16_t, false>(buffer, {0x0102_u16, 0x0304_u16, 0x0506_u16, 0x0708_u16, 0x090A_u16, 0x0B0C_u16, 0x0D0E_u16, 0x0F00_u16, 0x0000_u16});
    }
    { // if (bits == total_bits_T_val)
        uint32_t buffer[5] = {};
        serdes::bitcpy(serdes::sized_pointer<uint32_t>(buffer), x);
        test_cmp_arrays<uint32_t, false>(buffer, {0x01020304_u32, 0x05060708_u32, 0x090A0B0C_u32, 0x0D0E0F00_u32, 0x00000000_u32});
    }
    { // if (bits == total_bits_T_val)
        uint32_t buffer[5] = {};
        serdes::bitcpy(serdes::sized_pointer<uint32_t>(buffer), x, 4, sizeof(x) * 8);
        test_cmp_arrays<uint32_t, false>(buffer, {0x00102030_u32, 0x40506070_u32, 0x8090A0B0_u32, 0xC0D0E0F0_u32, 0x00000000_u32});
    }
    { // else if (bits < total_bits_T_val)
        uint32_t buffer[5] = {};
        serdes::bitcpy(serdes::sized_pointer<uint32_t>(buffer), x, 0, sizeof(x) * 8 - 16);
        test_cmp_arrays<uint32_t, false>(buffer, {0x03040506_u32, 0x0708090A_u32, 0x0B0C0D0E_u32, 0x0F000000_u32, 0x00000000_u32});
    }
    { // else if (bits < total_bits_T_val)
        uint32_t buffer[5] = {};
        serdes::bitcpy(serdes::sized_pointer<uint32_t>(buffer), x, 4, sizeof(x) * 8 - 16);
        test_cmp_arrays<uint32_t, false>(buffer, {0x00304050_u32, 0x60708090_u32, 0xA0B0C0D0_u32, 0xE0F00000_u32, 0x00000000_u32});
    }
    { // if (bits == total_bits_T_val)
        uint64_t buffer[3] = {};
        serdes::bitcpy(serdes::sized_pointer<uint64_t>(buffer), x);
        test_cmp_arrays<uint64_t, false>(buffer, {0x0102030405060708_u64, 0x090A0B0C0D0E0F00_u64, 0x0000000000000000_u64});
    }

    { // if (bits > total_bits_T_val)
        uint32_t buffer[5] = {};
        serdes::bitcpy(serdes::sized_pointer<uint32_t>(buffer), x, 0, sizeof(x) * 8 + 32);
        test_cmp_arrays<uint32_t, false>(buffer, {0x00000000_u32, 0x01020304_u32, 0x05060708_u32, 0x090A0B0C_u32, 0x0D0E0F00_u32});
    }
    { // if (bits > total_bits_T_val)
        uint32_t buffer[5] = {};
        serdes::bitcpy(serdes::sized_pointer<uint32_t>(buffer), x, 0, sizeof(x) * 8 + 20);
        test_cmp_arrays<uint32_t, false>(buffer, {0x00000010_u32, 0x20304050_u32, 0x60708090_u32, 0xA0B0C0D0_u32, 0xE0F00000_u32});
    }
    { // if (bits > total_bits_T_val)
        uint32_t buffer[5] = {};
        serdes::bitcpy(serdes::sized_pointer<uint32_t>(buffer), x, 0, sizeof(x) * 8 + 4);
        test_cmp_arrays<uint32_t, false>(buffer, {0x00102030_u32, 0x40506070_u32, 0x8090A0B0_u32, 0xC0D0E0F0_u32, 0x00000000_u32});
    }
    { // if (bits > total_bits_T_val)
        uint8_t buffer[17] = {0xFF, 0xFF};
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), x, 4, sizeof(x) * 8 + 4);
        test_cmp_arrays<uint8_t, false>(buffer, {0xF0_u8, 0x01_u8, 0x02_u8, 0x03_u8, 0x04_u8, 0x05_u8, 0x06_u8, 0x07_u8, 0x08_u8, 0x09_u8, 0x0A_u8, 0x0B_u8, 0x0C_u8, 0x0D_u8, 0x0E_u8, 0x0F_u8, 0x00_u8});
    }
}

static void test_to_array_sized_pointers()
{
    {
        uint8_t buffer[9] = {};
        uint32_t x = 0x0123ABCD;
        size_t bits_written = serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer, 8), x, 8 * 8 - 4, 20);
        test_cmp_arrays<uint8_t, false>(buffer, {0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8});
        ASSERT_EQUALS(bits_written, size_t(0));
        bits_written = serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer, 8), x, 8 * 8 - 4, 4);
        test_cmp_arrays<uint8_t, false>(buffer, {0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x0D_u8, 0x00_u8});
        ASSERT_EQUALS(bits_written, size_t(4));
        bits_written = serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer, 9), x, 8 * 8 - 4, 8 + 4);
        test_cmp_arrays<uint8_t, false>(buffer, {0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x0B_u8, 0xCD_u8});
        ASSERT_EQUALS(bits_written, size_t(12));
        bits_written = serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer, 9), x, 8 * 8 - 4, 8 + 5);
        test_cmp_arrays<uint8_t, false>(buffer, {0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x0B_u8, 0xCD_u8});
        ASSERT_EQUALS(bits_written, size_t(0));
    }
    {
        uint16_t buffer[6] = {0xFFFF, 0, 0, 0, 0xFFFF, 0};
        uint32_t x = 0x0123ABCD;
        size_t bits_written = serdes::bitcpy(serdes::sized_pointer<uint16_t>(buffer, 4), x, 8 * 8 - 4, 20);
        ASSERT_EQUALS(bits_written, size_t(0));
        test_cmp_arrays<uint16_t, false>(buffer, {0xFFFF_u16, 0x0000_u16, 0x0000_u16, 0x0000_u16, 0xFFFF_u16, 0x0000_u16});
        serdes::bitcpy(serdes::sized_pointer<uint16_t>(buffer, 6), x, 8 * 8 - 4, 20);
        test_cmp_arrays<uint16_t, false>(buffer, {0xFFFF_u16, 0x0000_u16, 0x0000_u16, 0x0003_u16, 0xABCD_u16, 0x0000_u16});
    }
}

static void test_to_array_pointers()
{
    uintptr_t buffer[3] = {};
    float x = 1.1234;
    float *y = &x;
    uintptr_t px = reinterpret_cast<uintptr_t>(&x);
    serdes::bitcpy(serdes::sized_pointer<uintptr_t>(buffer), y);
    test_cmp_arrays<uintptr_t, false>(buffer, {px, uintptr_t(0), uintptr_t(0)});
}

static void test_array_to_array()
{
    uintptr_t buffer[3] = {};
    float x = 1.1234;
    float *y = &x;
    uintptr_t px = reinterpret_cast<uintptr_t>(&x);
    serdes::bitcpy(serdes::sized_pointer<uintptr_t>(buffer), y);
    test_cmp_arrays<uintptr_t, false>(buffer, {px, uintptr_t(0), uintptr_t(0)});
}

static void test_to_array_zero_bits()
{
    uint16_t x = 123;
    uint8_t buff[] = {1, 2, 3};
    serdes::bitcpy(buff, x, 1u, 0u);
    test_cmp_arrays<uint8_t, false>(buff, {1_u8, 2_u8, 3_u8});
}

static void testset_to_array()
{
    // == bit sized tests with aligned offsets
    nominal_testing<uint8_t>();
    nominal_testing<uint16_t>();
    nominal_testing<uint32_t>();
    nominal_testing<uint64_t>();
    BITCPY_INT128_CONDITIONAL_DEFINE_C(
        nominal_testing<__uint128_t>(););

    // <= bit lengths, but within a single array item bit boundary
    underflow_under_array_boundary_testing<uint8_t>();
    underflow_under_array_boundary_testing<uint16_t>();
    underflow_under_array_boundary_testing<uint32_t>();
    underflow_under_array_boundary_testing<uint64_t>();
    BITCPY_INT128_CONDITIONAL_DEFINE_C(
        underflow_under_array_boundary_testing<__uint128_t>(););

    // <= bit lengths, but across a maximum of two array item bit boundaries
    underflow_over_array_boundary_testing<uint8_t>();
    underflow_over_array_boundary_testing<uint16_t>();
    underflow_over_array_boundary_testing<uint32_t>();
    underflow_over_array_boundary_testing<uint64_t>();
    BITCPY_INT128_CONDITIONAL_DEFINE_C(
        underflow_over_array_boundary_testing<__uint128_t>(););

    // > bit lengths, and across a more than two array item bit boundaries
    overflow_over_array_boundary_testing();

    // other type tests
    test_to_array_booleans();
    test_to_array_enums();
    test_to_array_large_types();
    test_to_array_floating_point();
    test_to_array_signed();
    test_to_array_pointers();
    test_to_array_sized_pointers();
    test_array_to_array();
    test_to_array_zero_bits();
}
#ifndef DISBALE_TESTS_MAIN
int main()
{
    testset_to_array();
    PRINT_SUMMARY();
}
#endif