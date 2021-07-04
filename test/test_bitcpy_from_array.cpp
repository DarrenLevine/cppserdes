#ifndef _TEST_BITCPY_FROM_ARRAY_H_
#define _TEST_BITCPY_FROM_ARRAY_H_

#include "test_utilities.h"

#ifdef __SIZEOF_INT128__
__uint128_t test_fa_array[] = {
    form_uint128_t(0xabcdef1200112233, 0xBBBBAAAAEFBD7D78),
    form_uint128_t(0xF111232444556677, 0xB26BA9AAEF0DBD4B),
    form_uint128_t(0x98765432A88EFCCA, 0x7B6BA5AAEFB8B92A),
    form_uint128_t(0x60cd601200412245, 0x9B41A1A4EFB47D16),
    form_uint128_t(0x7211202434511476, 0x8263A4AAEF01BD47),
    form_uint128_t(0x807651321134FC44, 0x016BA541EF48B426),
    form_uint128_t(0xEFcdef1200212233, 0xBEFBAAAAEFBD2DEF),
    form_uint128_t(0x12112326BB566677, 0xB12BA9AAEF0D5D12),
    form_uint128_t(0x117654326B8E4CCA, 0x711BA5AAEFB88911),
    form_uint128_t(0x64cdEF126B412245, 0x9B41A1BBEFB44D16),
    form_uint128_t(0x7611122434001476, 0x8263A46BEF010D47),
    form_uint128_t(0x857611321134F044, 0x016BA56BEF483426),
    form_uint128_t(0x64234cd124524050, 0x9B4541A1A4AEF0D1),
    form_uint128_t(0x7611143440014547, 0x824563A46BEF0104),
    form_uint128_t(0x823457611344F044, 0x016B45A56BE345F4),
    form_uint128_t(0x60cd601200412245, 0x9B41A1A4EFB47D16),
    form_uint128_t(0x153425B554645677, 0x9786635347579833)};
#else
uint64_t test_fa_array[] = {
    0xabcdef1200112233, 0xBBBBAAAAEFBD7D78,
    0xF111232444556677, 0xB26BA9AAEF0DBD4B,
    0x98765432A88EFCCA, 0x7B6BA5AAEFB8B92A,
    0x60cd601200412245, 0x9B41A1A4EFB47D16,
    0x7211202434511476, 0x8263A4AAEF01BD47,
    0x807651321134FC44, 0x016BA541EF48B426,
    0xEFcdef1200212233, 0xBEFBAAAAEFBD2DEF,
    0x12112326BB566677, 0xB12BA9AAEF0D5D12,
    0x117654326B8E4CCA, 0x711BA5AAEFB88911,
    0x64cdEF126B412245, 0x9B41A1BBEFB44D16,
    0x7611122434001476, 0x8263A46BEF010D47,
    0x857611321134F044, 0x016BA56BEF483426,
    0x64234cd124524050, 0x9B4541A1A4AEF0D1,
    0x7611143440014547, 0x824563A46BEF0104,
    0x823457611344F044, 0x016B45A56BE345F4,
    0x60cd601200412245, 0x9B41A1A4EFB47D16,
    0x153425B554645677, 0x9786635347579833};
#endif
const size_t array_size = sizeof(test_fa_array) / sizeof(test_fa_array[0]);

template <typename T_array, typename T_val>
void bitcpy_and_reverse(T_val &dest, const T_array source[array_size], const size_t bits, const size_t bit_offset = 0)
{
    T_array source_2[array_size] = {};
    for (size_t i = 0; i < array_size; i++)
        source_2[i] = source[i];
    dest = 0;
    serdes::bitcpy(dest, source, bit_offset, bits);
    T_val dest2 = dest + 1;
    if (bits <= sizeof(dest) * 8) // we can only do follow up tests if information isn't lost
    {
        serdes::bitcpy(source_2, dest2, bit_offset, bits);
        serdes::bitcpy(source_2, dest, bit_offset, bits);
        for (size_t i = 0; i < array_size; i++)
            ASSERT_EQUALS(source_2[i], source[i]);
    }
}

template <typename test_type>
static void test_from_array_aligned_sizes_and_offsets()
{
    test_type x[array_size] = {};
    for (size_t i = 0; i < array_size; i++)
        x[i] = static_cast<test_type>(test_fa_array[i]);
    test_type y = 0;
    size_t test_type_bits = sizeof(test_type) * 8u;

    bitcpy_and_reverse(y, x, test_type_bits, 0);
    ASSERT_EQUALS(y, x[0]);

    bitcpy_and_reverse(y, x, test_type_bits, test_type_bits);
    ASSERT_EQUALS(y, x[1]);

    typename serdes::bitprint_detail::make_signed<test_type>::type y2 = -124;
    bitcpy_and_reverse(y2, x, test_type_bits, test_type_bits * 2);
    ASSERT_EQUALS(y2, x[2]);
}

template <typename test_type>
static void test_from_array_non_overlapping_buffer_cpy()
{
    {
        test_type x[array_size] = {};
        for (size_t i = 0; i < array_size; i++)
            x[i] = static_cast<test_type>(test_fa_array[i]);
        size_t test_type_bits = sizeof(test_type) * 8u;
        test_type y = 0;

        bitcpy_and_reverse(y, x, test_type_bits - 8, 0);
        ASSERT_EQUALS(y, static_cast<test_type>(x[0] >> 8));

        bitcpy_and_reverse(y, x, test_type_bits - 4, 0);
        ASSERT_EQUALS(y, static_cast<test_type>(x[0] >> 4));

        bitcpy_and_reverse(y, x, test_type_bits - 8, test_type_bits);
        ASSERT_EQUALS(y, static_cast<test_type>(x[1] >> 8));

        typename serdes::bitprint_detail::make_signed<test_type>::type y2 = 0;
        bitcpy_and_reverse(y2, x, test_type_bits - 4, test_type_bits * 2);
        typename serdes::bitprint_detail::make_signed<test_type>::type expected_value = static_cast<test_type>(x[2] >> 4);
        serdes::detail::extend_sign(expected_value, test_type_bits - 4);
        ASSERT_EQUALS(y2, expected_value);
    }
    {

        uint16_t x[array_size] = {};
        for (size_t i = 0; i < array_size; i++)
            x[i] = static_cast<uint16_t>(test_fa_array[i]);
        size_t test_type_bits = sizeof(test_type) * 8u;

        test_type y = 123;
        serdes::bitcpy(y, x, 0, 0);
        ASSERT_EQUALS(y, static_cast<test_type>(123));
        serdes::bitcpy(y, x, 0, 8);
        ASSERT_EQUALS(y, static_cast<test_type>(x[0] >> 8));
        serdes::bitcpy(y, x, 0, 16);
        ASSERT_EQUALS(y, static_cast<test_type>(x[0]));
    }
}

template <typename test_type>
static void test_from_array_overlapping_buffer_cpy()
{
    uint16_t source_array[array_size] = {};
    for (size_t i = 0; i < array_size; i++)
        source_array[i] = static_cast<uint16_t>(test_fa_array[i]);
    size_t test_type_bits = sizeof(test_type) * 8u;
    const uint32_t first32bits = static_cast<uint32_t>(source_array[0]) << 16 | static_cast<uint32_t>(source_array[1]);
    const uint32_t second32bits = static_cast<uint32_t>(source_array[2]) << 16 | static_cast<uint32_t>(source_array[3]);
    const uint64_t first64bits = static_cast<uint64_t>(first32bits) << 32 | static_cast<uint64_t>(second32bits);

    test_type dest_value = 0;
    bitcpy_and_reverse(dest_value, source_array, 24, 2);
    ASSERT_EQUALS(dest_value, static_cast<test_type>(
                                  (first32bits >> (32 - 24 - 2)) & serdes::detail::bitmask<uint32_t>(24)));
    BITCPY_INT128_CONDITIONAL_DEFINE_C(
        bitcpy_and_reverse(dest_value, source_array, 68);
        ASSERT_EQUALS(dest_value, static_cast<test_type>(
                                      (static_cast<__uint128_t>(first64bits) << 4) |
                                      static_cast<__uint128_t>(source_array[4] >> (16 - 4)))););
    bitcpy_and_reverse(dest_value, source_array, 63);
    ASSERT_EQUALS(dest_value, static_cast<test_type>(first64bits >> 1));
    bitcpy_and_reverse(dest_value, source_array, 20, 36);
    ASSERT_EQUALS(dest_value, static_cast<test_type>((first64bits >> (64 - 36 - 20)) & serdes::detail::bitmask<uint64_t>(20)));

    bitcpy_and_reverse(dest_value, source_array, 32, 18);
    ASSERT_EQUALS(dest_value, static_cast<test_type>((first64bits >> (64 - 18 - 32)) & serdes::detail::bitmask<uint64_t>(32)));
}

static void test_from_array_booleans()
{
    uint64_t bool_test_array[5] = {};
    serdes::bitcpy(bool_test_array, true, 64 * 2 + 7);
    ASSERT_EQUALS(bool_test_array[2], uint64_t(1llu << (64 - 7 - 1)));

    serdes::bitcpy(bool_test_array, true, 64 * 2);
    ASSERT_EQUALS(bool_test_array[2], uint64_t(1llu << 63) | uint64_t(1llu << (64 - 7 - 1)));

    bool dest = false;
    serdes::bitcpy(dest, bool_test_array, 64 * 2 + 7);
    ASSERT_EQUALS(dest, true);
    serdes::bitcpy(dest, bool_test_array, 64 * 2 + 6);
    ASSERT_EQUALS(dest, false);
    serdes::bitcpy(dest, bool_test_array, 64 * 2);
    ASSERT_EQUALS(dest, true);
    serdes::bitcpy(dest, bool_test_array);
    ASSERT_EQUALS(dest, false);
    for (size_t i = 0; i < 64; i++)
    {
        bool_test_array[0] = static_cast<uint64_t>(1u) << (63 - i);
        serdes::bitcpy(dest, bool_test_array, i + 1);
        ASSERT_EQUALS(dest, false);
        serdes::bitcpy(dest, bool_test_array, i);
        ASSERT_EQUALS(dest, true);
    }
}

static void test_from_array_enums()
{
    enum class temp_e : uint32_t
    {
        QUICK,
        BROWN,
        FOX = 0x1234ABCD
    };
    temp_e recovered = temp_e::BROWN;

    {
        uint8_t buffer[7] = {};
        serdes::bitcpy(serdes::sized_pointer<uint8_t>(buffer), temp_e::FOX);
        const uint8_t *const b_pointer = &buffer[0]; // ensure const is supported
        const auto sp = serdes::sized_pointer<const uint8_t>(b_pointer, 7);
        serdes::bitcpy(recovered, sp);
        ASSERT_EQUALS(static_cast<uint32_t>(recovered), static_cast<uint32_t>(temp_e::FOX));
    }
    {
        uint64_t buffer[2] = {0x0000000000000ABC, 0xD000000000000000};
        serdes::bitcpy(recovered, serdes::sized_pointer<uint64_t>(buffer), 52, 16);
        ASSERT_EQUALS(static_cast<uint32_t>(recovered), static_cast<uint32_t>(static_cast<uint16_t>(temp_e::FOX)));
    }
}

static void test_from_array_large_types()
{
    struct large_class
    {
        uint8_t value[15] = {};
    };

    BITCPY_INT128_CONDITIONAL_DEFINE_C(
        {
            __uint128_t f[2] = {0xFF5056, 0xACDB9};
            large_class x;
            serdes::bitcpy(x, serdes::sized_pointer<__uint128_t>(f));
            test_cmp_arrays<uint8_t, false>(x.value, {0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0x00_u8, 0xFF_u8, 0x50_u8});
        });
    { //if (bits == total_bits_T_val)
        uint8_t buffer[17] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00};
        large_class x;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer));
        test_cmp_arrays<uint8_t, false>(x.value, {0x01_u8, 0x02_u8, 0x03_u8, 0x04_u8, 0x05_u8, 0x06_u8, 0x07_u8, 0x08_u8, 0x09_u8, 0x0A_u8, 0x0B_u8, 0x0C_u8, 0x0D_u8, 0x0E_u8, 0x0F_u8});
    }
    { //if (bits == total_bits_T_val)
        large_class x;
        uint16_t buffer[9] = {0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C, 0x0D0E, 0x0F00, 0x0000};
        serdes::bitcpy(x, serdes::sized_pointer<uint16_t>(buffer));
        test_cmp_arrays<uint8_t, false>(x.value, {0x01_u8, 0x02_u8, 0x03_u8, 0x04_u8, 0x05_u8, 0x06_u8, 0x07_u8, 0x08_u8, 0x09_u8, 0x0A_u8, 0x0B_u8, 0x0C_u8, 0x0D_u8, 0x0E_u8, 0x0F_u8});
    }
    { //if (bits == total_bits_T_val)
        large_class x;
        uint32_t buffer[5] = {0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F00, 0};
        serdes::bitcpy(x, serdes::sized_pointer<uint32_t>(buffer));
        test_cmp_arrays<uint8_t, false>(x.value, {0x01_u8, 0x02_u8, 0x03_u8, 0x04_u8, 0x05_u8, 0x06_u8, 0x07_u8, 0x08_u8, 0x09_u8, 0x0A_u8, 0x0B_u8, 0x0C_u8, 0x0D_u8, 0x0E_u8, 0x0F_u8});
    }
    { //if (bits == total_bits_T_val)
        large_class x;
        uint32_t buffer[5] = {0x00102030, 0x40506070, 0x8090A0B0, 0xC0D0E0F0, 0x00000000};
        serdes::bitcpy(x, serdes::sized_pointer<uint32_t>(buffer), 4, sizeof(x) * 8);
        test_cmp_arrays<uint8_t, false>(x.value, {0x01_u8, 0x02_u8, 0x03_u8, 0x04_u8, 0x05_u8, 0x06_u8, 0x07_u8, 0x08_u8, 0x09_u8, 0x0A_u8, 0x0B_u8, 0x0C_u8, 0x0D_u8, 0x0E_u8, 0x0F_u8});
    }
    { // else if (bits < total_bits_T_val)
        large_class x;
        uint32_t buffer[5] = {0xFA102030, 0x40506070, 0x8090A0B0, 0xC1D2E3F4, 0x11324458};
        serdes::bitcpy(x, serdes::sized_pointer<uint32_t>(buffer), 0, sizeof(x) * 8 - 16);
        test_cmp_arrays<uint8_t, false>(x.value, {0x00_u8, 0x00_u8, 0xFA_u8, 0x10_u8, 0x20_u8, 0x30_u8, 0x40_u8, 0x50_u8, 0x60_u8, 0x70_u8, 0x80_u8, 0x90_u8, 0xA0_u8, 0xB0_u8, 0xC1_u8});
    }
    { // else if (bits < total_bits_T_val)
        large_class x;
        uint32_t buffer[5] = {0xFA102030, 0x40506070, 0x8090A0B0, 0xC1D2E3F4, 0x11324458};
        serdes::bitcpy(x, serdes::sized_pointer<uint32_t>(buffer), 4, sizeof(x) * 8 - 16);
        test_cmp_arrays<uint8_t, false>(x.value, {0x00_u8, 0x00_u8, 0xA1_u8, 0x02_u8, 0x03_u8, 0x04_u8, 0x05_u8, 0x06_u8, 0x07_u8, 0x08_u8, 0x09_u8, 0x0A_u8, 0x0B_u8, 0x0C_u8, 0x1D_u8});
    }
    { //if (bits == total_bits_T_val)
        large_class x;
        uint64_t buffer[3] = {0xFA10203040506070, 0x8090A0B0C1D2E3F4, 0x11324458D1EF5324};
        serdes::bitcpy(x, serdes::sized_pointer<uint64_t>(buffer));
        test_cmp_arrays<uint8_t, false>(x.value, {0xFA_u8, 0x10_u8, 0x20_u8, 0x30_u8, 0x40_u8, 0x50_u8, 0x60_u8, 0x70_u8, 0x80_u8, 0x90_u8, 0xA0_u8, 0xB0_u8, 0xC1_u8, 0xD2_u8, 0xE3_u8});
    }

    { //if (bits > total_bits_T_val)
        large_class x;
        uint32_t buffer[5] = {0xFA102030, 0x40506070, 0x8090A0B0, 0xC1D2E3F4, 0x11324458};
        serdes::bitcpy(x, serdes::sized_pointer<uint32_t>(buffer), 0, sizeof(x) * 8 + 32);
        test_cmp_arrays<uint8_t, false>(x.value, {0x40_u8, 0x50_u8, 0x60_u8, 0x70_u8, 0x80_u8, 0x90_u8, 0xA0_u8, 0xB0_u8, 0xC1_u8, 0xD2_u8, 0xE3_u8, 0xF4_u8, 0x11_u8, 0x32_u8, 0x44_u8});
    }
    { //if (bits > total_bits_T_val)
        large_class x;
        uint32_t buffer[5] = {0xFA162D3E, 0x42506070, 0x8090A0B0, 0xC1D2E3F4, 0x11324458};
        serdes::bitcpy(x, serdes::sized_pointer<uint32_t>(buffer), 0, sizeof(x) * 8 + 20);
        test_cmp_arrays<uint8_t, false>(x.value, {0xD3_u8, 0xE4_u8, 0x25_u8, 0x06_u8, 0x07_u8, 0x08_u8, 0x09_u8, 0x0A_u8, 0x0B_u8, 0x0C_u8, 0x1D_u8, 0x2E_u8, 0x3F_u8, 0x41_u8, 0x13_u8});
    }
    { //if (bits > total_bits_T_val)
        large_class x;
        uint32_t buffer[5] = {0xFA162D3E, 0x42506070, 0x8090A0B0, 0xC1D2E3F4, 0x11324458};
        serdes::bitcpy(x, serdes::sized_pointer<uint32_t>(buffer), 0, sizeof(x) * 8 + 4);
        test_cmp_arrays<uint8_t, false>(x.value, {0xA1_u8, 0x62_u8, 0xD3_u8, 0xE4_u8, 0x25_u8, 0x06_u8, 0x07_u8, 0x08_u8, 0x09_u8, 0x0A_u8, 0x0B_u8, 0x0C_u8, 0x1D_u8, 0x2E_u8, 0x3F_u8});
    }
    { //if (bits > total_bits_T_val)
        large_class x;
        uint8_t buffer[17] = {0xFE, 0xB2, 0xA3, 0xE4, 0xF5, 0x16, 0x27, 0x38, 0x049, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x00, 0x00};
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer), 4, sizeof(x) * 8 + 8);
        test_cmp_arrays<uint8_t, false>(x.value, {0x2A_u8, 0x3E_u8, 0x4F_u8, 0x51_u8, 0x62_u8, 0x73_u8, 0x84_u8, 0x90_u8, 0xA0_u8, 0xB0_u8, 0xC0_u8, 0xD0_u8, 0xE0_u8, 0xF0_u8, 0x00_u8});
    }
}

static void test_from_array_floating_point()
{
    {
        uint8_t buffer[9] = {0xDA, 0xE0, 0x62, 0xA7, 0x65, 0x8C, 0xDA, 0x16, 0x00};
        double x = 0.0f;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer));
        ASSERT_EQUALS(x, -56789.01234E125);
    }
    {
        uint8_t buffer[8] = {0x3F, 0x8F, 0xCB, 0x92, 0x00, 0x00, 0x00, 0x00};
        float x = 0.0f;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer));
        ASSERT_EQUALS(x, 1.1234f);
    }
    {
        uint8_t buffer[8] = {0x03, 0xF8, 0xFC, 0xB9, 0x20, 0x00, 0x00, 0x00};
        float x = 0.0f;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer), 4, sizeof(x) * 8);
        ASSERT_EQUALS(x, 1.1234f);
    }
    {
        uint8_t buffer[8] = {0xF8, 0xFC, 0xB9, 0x20, 0xDA, 0x14, 0x67, 0x98};
        float x = 0.0f;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer), 4, sizeof(x) * 8 + 8);
        ASSERT_EQUALS(x, -19143490.f);
    }
    {
        uint8_t buffer[8] = {0xF8, 0xFC, 0xB9, 0x20, 0xDA, 0x14, 0x67, 0x98};
        float x = 0.0f;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer), 9, sizeof(x) * 8 - 1);
        ASSERT_EQUALS(x, 7689929821405504594858428204224348160.f);
    }
}

static void test_from_array_signed()
{
    {
        uint8_t buffer[8] = {0xFF, 0xFF, 0xFF, 0xFC, 0x00, 0x00, 0x00, 0x00};
        int32_t x = 0;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer));
        ASSERT_EQUALS(x, int32_t(-4));
    }
    {
        uint8_t buffer[8] = {0x0F, 0xFF, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00};
        int32_t x = 0;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer), 4, 32);
        ASSERT_EQUALS(x, int32_t(-4));
    }
    {
        uint8_t buffer[8] = {0x0F, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00};
        int32_t x = 0;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer), 4, 16);
        ASSERT_EQUALS(x, int32_t(-4));
    }
    { //  tests sign extension
        uint8_t buffer[8] = {0x00, 0x80, 0xAB, 0xCB, 0xEF, 0x00, 0x00, 0x00};
        int32_t x = 0;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer), 8, 16);
        ASSERT_EQUALS(x, int32_t(0xFFFF80AB));
    }
}
static void test_from_array_pointers()
{
    float x = 1.1234;
    float *y = nullptr;
    uintptr_t px = reinterpret_cast<uintptr_t>(&x);
    uintptr_t buffer[3] = {px, 0, 0};
    serdes::bitcpy(y, serdes::sized_pointer<uintptr_t>(buffer));
    ASSERT_EQUALS(reinterpret_cast<uintptr_t>(y), reinterpret_cast<uintptr_t>(px));
}
static void test_from_array_sized_pointers()
{
    {
        uint8_t buffer[9] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint32_t x = 0x0123ABCD;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer, 8), 8 * 8 - 4, 20);
        ASSERT_EQUALS(x, 0x0123ABCD);
    }
    {
        uint8_t buffer[9] = {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0xF6, 0x12};
        uint32_t x = 0;
        auto bits_written = serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer, 9), 8 * 7, 17);
        ASSERT_EQUALS(x, uint32_t(0));          // no overflow allowed
        ASSERT_EQUALS(bits_written, size_t(0)); // no overflow allowed
        bits_written = serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer, 9), 8 * 7, 16);
        ASSERT_EQUALS(x, uint32_t(0xF612));
        ASSERT_EQUALS(bits_written, size_t(16));
    }
    {
        uint8_t buffer[11] = {0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0xF6, 0x12, 0xAE, 0x67};
        uint32_t x = 0;
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer, 9), 8 * 7, 20);
        ASSERT_EQUALS(x, uint32_t(0));
        serdes::bitcpy(x, serdes::sized_pointer<uint8_t>(buffer, 8), 8 * 7, 8);
        ASSERT_EQUALS(x, uint32_t(0xF6));
    }
    {
        uint16_t buffer[6] = {0xFFFF, 0, 0, 0xABCD, 0x1234, 0};
        uint32_t x = 0x0123ABCD;
        serdes::bitcpy(x, serdes::sized_pointer<uint16_t>(buffer, 4), 4 * 16 - 4, 4);
        ASSERT_EQUALS(x, uint32_t(0xD));
        serdes::bitcpy(x, serdes::sized_pointer<uint16_t>(buffer, 6), 4 * 16 - 4, 20);
        ASSERT_EQUALS(x, uint32_t(0xD1234));
        serdes::bitcpy(x, serdes::sized_pointer<uint16_t>(buffer, 6), 4 * 16 - 4, 16 * 2);
        ASSERT_EQUALS(x, uint32_t(0xD1234000));
        x = 0;
        serdes::bitcpy(x, serdes::sized_pointer<uint16_t>(buffer, 6), 4 * 16 - 4, 16 * 2 + 5);
        ASSERT_EQUALS(x, uint32_t(0)); // overflow case
    }
}

static void test_from_array_zero_bits()
{
    uint16_t x = 123;
    uint8_t buff[] = {1, 2, 3};
    serdes::bitcpy(x, buff, 1u, 0u);
    ASSERT_EQUALS(x, uint16_t(123));
}

static void testset_from_array()
{
    run_type_tests(test_from_array_aligned_sizes_and_offsets);
    run_type_tests(test_from_array_non_overlapping_buffer_cpy);
    run_type_tests(test_from_array_overlapping_buffer_cpy);

    // other type tests
    test_from_array_booleans();
    test_from_array_enums();
    test_from_array_large_types();
    test_from_array_floating_point();
    test_from_array_signed();
    test_from_array_pointers();
    test_from_array_sized_pointers();
    test_from_array_zero_bits();
}

#ifndef DISBALE_TESTS_MAIN
int main()
{
    testset_from_array();
    PRINT_SUMMARY();
}
#endif

#endif // _TEST_BITCPY_FROM_ARRAY_H_
