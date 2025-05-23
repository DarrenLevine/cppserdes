#include "../test/test_utilities.h"
#include <array>

static void test_std_array()
{
    {
        std::array<uint8_t, 3> source{{0xAB_u8, 0xCD_u8, 0xEF_u8}};
        std::array<uint8_t, 3> target{};

        // test std::array into bytes as a std::array
        serdes::packet(target) << source;
        ASSERT_EQUALS(source, target);

        // test std::array out of bytes as a std::array
        source = std::array<uint8_t, 3>{{0x01_u8, 0x23_u8, 0x45_u8}};
        serdes::packet(target) >> source;
        ASSERT_EQUALS(source, target);
    }
    {
        char source_underlying[13] = "hello world!";
        serdes::packet source(source_underlying);

        uint8_t target[13] = {1_u8, 2_u8, 3_u8};
        ASSERT_EQUALS(target, {1_u8, 2_u8, 3_u8});
        source >> target;
        ASSERT_EQUALS(source_underlying, target);
    }
    for (size_t size = 0; size < 5; size++)
    {
        std::array<uint8_t, 3> source{{0xAB_u8, 0xCD_u8, 0xEF_u8}};
        std::array<uint8_t, 3> target{};
        serdes::packet target_pkt(target);
        target_pkt << serdes::array<std::array<uint8_t, 3>, size_t>(source, size);
        if (size > target.size())
            ASSERT_EQUALS(static_cast<int>(target_pkt.status), static_cast<int>(serdes::status_e::ARRAY_SIZE_OVER_MAX));
        else
            ASSERT_EQUALS(static_cast<int>(target_pkt.status), static_cast<int>(serdes::status_e::NO_ERROR));
    }

    // test dynamic sizes with std::array<uint8_t>
    {
        std::array<uint8_t, 100> source{{0xAB_u8, 0xCD_u8, 0xEF_u8}};
        std::array<uint8_t, 1000> target{};

        volatile size_t dyn_size = 3;
        // test std::array into bytes as a std::array
        {
            serdes::packet bytes(target);
            bytes.mode = serdes::mode_e::STORING;
            bytes + serdes::array<std::array<uint8_t, 100UL>, volatile size_t>(source, dyn_size);
            ASSERT_EQUALS(target, source);
            ASSERT_EQUALS(bytes.bit_offset, dyn_size*8);
            ASSERT_EQUALS(static_cast<int>(bytes.status), static_cast<int>(serdes::status_e::NO_ERROR));
        }

        // test std::array out of bytes as a std::array
        source = std::array<uint8_t, 100>{{0x01_u8, 0x23_u8, 0x45_u8}}; // corrupt source
        {
            serdes::packet bytes(target);
            bytes.mode = serdes::mode_e::LOADING;
            bytes + serdes::array<std::array<uint8_t, 100UL>, volatile size_t>(source, dyn_size);
            ASSERT_EQUALS(target, source);
            ASSERT_EQUALS(bytes.bit_offset, dyn_size*8);
            ASSERT_EQUALS(static_cast<int>(bytes.status), static_cast<int>(serdes::status_e::NO_ERROR));
        }
    }

    // test dynamic sizes with std::array<uint16_t>
    {
        std::array<uint16_t, 50> source{{0xABCD_u16, 0xEF01_u16, 0x2345_u16}};
        std::array<uint8_t, 1000> target{};

        volatile size_t dyn_size = 2;
        // test std::array into bytes as a std::array
        {
            serdes::packet bytes(target);
            bytes << serdes::array<std::array<uint16_t, 50UL>, volatile size_t>(source, dyn_size);
            ASSERT_EQUALS(target[0], 0xAB_u8);
            ASSERT_EQUALS(target[1], 0xCD_u8);
            ASSERT_EQUALS(target[2], 0xEF_u8);
            ASSERT_EQUALS(target[3], 0x01_u8);
            ASSERT_EQUALS(target[4], 0x00_u8);
            ASSERT_EQUALS(bytes.bit_offset, dyn_size*16);
            ASSERT_EQUALS(static_cast<int>(bytes.status), static_cast<int>(serdes::status_e::NO_ERROR));
        }

        // test std::array out of bytes as a std::array
        source = std::array<uint16_t, 50>{{0x0000_u16, 0x0000_u16, 0x0000_u16}}; // corrupt source
        {
            serdes::packet bytes(target);
            bytes >> serdes::array<std::array<uint16_t, 50UL>, volatile size_t>(source, dyn_size);
            ASSERT_EQUALS(source[0], 0xABCD_u16);
            ASSERT_EQUALS(source[1], 0xEF01_u16);
            ASSERT_EQUALS(source[2], 0x0000_u16);
            ASSERT_EQUALS(bytes.bit_offset, dyn_size*16);
            ASSERT_EQUALS(static_cast<int>(bytes.status), static_cast<int>(serdes::status_e::NO_ERROR));
        }
    }
}
static void testset_custom_types()
{
    test_std_array();
}

#ifndef DISBALE_TESTS_MAIN
int main()
{
    testset_custom_types();
    PRINT_SUMMARY();
}
#endif