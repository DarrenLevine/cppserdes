#include "../test/test_utilities.h"

static void test_variable_arrays()
{
    using test_array_t = serdes::array<uint8_t, uint8_t>;
    {
        uint8_t length = 3;
        uint8_t array[] = {0xAB, 0xCD, 0xEF, 0x12, 0x23};
        uint16_t serial_data[4] = {};
        serdes::packet(serial_data) << test_array_t(array, length);
        ASSERT_EQUALS(serial_data, {0xABCD, 0xEF00, 0x0000});
        std::fill(array, array + sizeof(array), 0xFF);
        serdes::packet(serial_data) >> test_array_t(array, length);
        ASSERT_EQUALS(array, {0xAB, 0xCD, 0xEF, 0xFF, 0xFF});
    }
    {
        uint8_t array[] = {0xAB, 0xCD, 0xEF, 0x12, 0x23};
        uint8_t length = sizeof(array) / sizeof(array[0]) + 1; // force an overrun
        uint16_t serial_data[6] = {};
        auto store_result = (serdes::packet(serial_data) << test_array_t(array, length));
        ASSERT_EQUALS(static_cast<int>(store_result.status), static_cast<int>(serdes::status_e::ARRAY_SIZE_OVER_MAX));
        ASSERT_EQUALS(store_result.bit_offset, (length - 1) * 8);
        ASSERT_EQUALS(serial_data, {0xABCD, 0xEF12, 0x2300, 0x0000});

        std::fill(array, array + sizeof(array), 0xFF);
        auto load_result = (serdes::packet(serial_data) >> test_array_t(array, length));
        ASSERT_EQUALS(static_cast<int>(load_result.status), static_cast<int>(serdes::status_e::ARRAY_SIZE_OVER_MAX));
        ASSERT_EQUALS(load_result.bit_offset, (length - 1) * 8);
        ASSERT_EQUALS(array, {0xAB, 0xCD, 0xEF, 0x12, 0x23});
    }
    {
        struct my_info : serdes::packet_base
        {
            uint8_t length = 3;
            uint8_t data[5] = {0xAB, 0xCD, 0xEF, 0x12, 0x23};

            void format(serdes::packet &p)
            {
                p + length + test_array_t(data, length) + test_array_t(data + 3, 2, 2); // variable length, sized erased
            }
        };
        my_info m;
        uint16_t serial_data[4] = {};
        m.store(serial_data);
        ASSERT_EQUALS(serial_data, {0x03AB, 0xCDEF, 0x1223, 0x0000});
        m.length = 0;
        std::fill(m.data, m.data + sizeof(m.data), 0xFF);
        m.load(serial_data);
        ASSERT_EQUALS(m.length, 3);
        ASSERT_EQUALS(m.data, {0xAB, 0xCD, 0xEF, 0x12, 0x23});
    }
}

static void test_variable_packet_base_arrays()
{
    {
        struct coordinates : serdes::packet_base
        {
            uint8_t x = 0xAB, y = 0xCD, z = 0xEF, q = 0x12;
            void format(serdes::packet &p) override
            {
                p + x + y + z + q;
            }
        };
        struct my_info : serdes::packet_base
        {
            int16_t length = 3;
            coordinates data[5] = {};

            void format(serdes::packet &p)
            {
                p + length + serdes::array<coordinates, int16_t>(data, length);
            }
        };
        my_info m;
        uint32_t serial_data[5] = {};
        m.store(serial_data);
        ASSERT_EQUALS(serial_data, {0x0003ABCD_u32, 0xEF12ABCD_u32, 0xEF12ABCD_u32, 0xEF120000_u32, 0x00000000_u32});
        m.length = 0;
        for (size_t i = 0; i < sizeof(m.data) / sizeof(m.data[0]); i++)
        {
            m.data[i].x = i;
            m.data[i].y = i + 1;
            m.data[i].z = i + 2;
            m.data[i].q = i + 3;
        }

        m.load(serial_data);
        ASSERT_EQUALS(m.length, 3);
        for (size_t i = 0; i < static_cast<size_t>(m.length); i++)
        {
            ASSERT_EQUALS(m.data[i].x, 0xAB_u8);
            ASSERT_EQUALS(m.data[i].y, 0xCD_u8);
            ASSERT_EQUALS(m.data[i].z, 0xEF_u8);
            ASSERT_EQUALS(m.data[i].q, 0x12_u8);
        }
        for (size_t i = static_cast<size_t>(m.length); i < sizeof(m.data) / sizeof(m.data[0]); i++)
        {
            ASSERT_EQUALS(m.data[i].x, uint8_t(i + 0));
            ASSERT_EQUALS(m.data[i].y, uint8_t(i + 1));
            ASSERT_EQUALS(m.data[i].z, uint8_t(i + 2));
            ASSERT_EQUALS(m.data[i].q, uint8_t(i + 3));
        }
    }
}
static void test_fixed_sized_arrays()
{
    {
        uint8_t array[] = {0xAB, 0xCD, 0xEF, 0x12, 0x23};
        uint16_t serial_data[4] = {};
        serdes::packet(serial_data) << array;
        ASSERT_EQUALS(serial_data, {0xABCD, 0xEF12, 0x2300});
        std::fill(array, array + sizeof(array), 0xFF);
        serdes::packet(serial_data) >> array;
        ASSERT_EQUALS(array, {0xAB, 0xCD, 0xEF, 0x12, 0x23});
    }
    {
        struct my_info : serdes::packet_base
        {
            uint8_t length = 3;
            uint8_t data[5] = {0xAB, 0xCD, 0xEF, 0x12, 0x23};
            int16_t data2 = 0x2345;

            void format(serdes::packet &p)
            {
                p + length + data + data2;
            }
        };
        my_info m;
        uint16_t serial_data[4] = {};
        m.store(serial_data);
        ASSERT_EQUALS(serial_data, {0x03AB, 0xCDEF, 0x1223, 0x2345});
        m.length = 0;
        m.data2 = 9;
        std::fill(m.data, m.data + sizeof(m.data), 0xFF);
        m.load(serial_data);
        ASSERT_EQUALS(m.length, 3);
        ASSERT_EQUALS(m.data, {0xAB, 0xCD, 0xEF, 0x12, 0x23});
        ASSERT_EQUALS(m.data2, 0x2345);
    }
}

static void test_bitpacked_arrays()
{
    uint64_t AB_flags[8] = {1, 0, 1, 0, 1, 0, 1, 1};
    uint64_t CD_flags[8] = {1, 1, 0, 0, 1, 1, 0, 1};
    int array_length = 8;
    using test_array_t = serdes::array<uint64_t, int>;
    uint16_t serial_data[4] = {};
    serdes::packet(serial_data) << serdes::bitpack<test_array_t, int>(test_array_t(AB_flags, array_length), 1) << serdes::bitpack<uint64_t[8], size_t>(CD_flags, 1);
    ASSERT_EQUALS(serial_data, {0xABCD, 0x0000});
    std::fill(AB_flags, AB_flags + 8, 0xF0);
    serdes::packet(serial_data) >> serdes::bitpack<test_array_t, int>(test_array_t(AB_flags, array_length), 1) << serdes::bitpack<uint64_t[8], size_t>(CD_flags, 1);
    ASSERT_EQUALS(AB_flags, {1, 0, 1, 0, 1, 0, 1, 1});
    ASSERT_EQUALS(CD_flags, {1, 1, 0, 0, 1, 1, 0, 1});
}

static void test_dynamic_bitlength_captures()
{
    struct my_info : serdes::packet_base
    {
        int8_t bits_in_time_tag = 0;
        uint16_t time_tag = 0;
        int8_t bits_in_time_tag2 = 0;
        uint16_t time_tag2 = 0;

        void format(serdes::packet &archive)
        {
            archive +
                bits_in_time_tag +
                serdes::bitpack<uint16_t, int8_t>(time_tag, bits_in_time_tag) +
                bits_in_time_tag2 +
                serdes::bitpack<uint16_t, int8_t>(time_tag2, serdes::bit_length(bits_in_time_tag2));
        }
    };
    uint16_t serial_data[] = {0x4B0, 0x8AB0};
    my_info object;
    object.load(serial_data);
    ASSERT_EQUALS(object.bits_in_time_tag, 4_i8);
    ASSERT_EQUALS(object.time_tag, 0xB_u16);
    ASSERT_EQUALS(object.bits_in_time_tag2, 8_i8);
    ASSERT_EQUALS(object.time_tag2, 0xAB_u16);
}

static void test_aligned_byte_arrays()
{
    {
        uint8_t length = 3;
        uint8_t array[] = {0xAB, 0xCD, 0xEF, 0x12, 0x23};
        uint8_t serial_data[6] = {};
        serdes::packet(serial_data) << length << array;
        ASSERT_EQUALS(serial_data, {0x3, 0xAB, 0xCD, 0xEF, 0x12, 0x23});
        length = 100;
        std::fill(array, array + sizeof(array), 0xFF);
        serdes::packet(serial_data) >> length >> array;
        ASSERT_EQUALS(array, {0xAB, 0xCD, 0xEF, 0x12, 0x23});
        ASSERT_EQUALS(length, 3);
    }
    {
        struct my_info : serdes::packet_base
        {
            uint8_t length = 3;
            uint8_t data[5] = {0xAB, 0xCD, 0xEF, 0x12, 0x23};
            int16_t data2 = 0x2345;

            void format(serdes::packet &p)
            {
                p + length + data + data2;
            }
        };
        my_info m;
        uint8_t serial_data[8] = {};
        m.store(serial_data);
        ASSERT_EQUALS(serial_data, {0x03, 0xAB, 0xCD, 0xEF, 0x12, 0x23, 0x23, 0x45});
        m.length = 0;
        m.data2 = 9;
        std::fill(m.data, m.data + sizeof(m.data), 0xFF);
        m.load(serial_data);
        ASSERT_EQUALS(m.length, 3);
        ASSERT_EQUALS(m.data, {0xAB, 0xCD, 0xEF, 0x12, 0x23});
        ASSERT_EQUALS(m.data2, 0x2345);
    }
}

static void test_inheritance_nesting()
{
    struct header_type : serdes::packet_base
    {
        uint8_t id = 9;
        uint16_t length = 1;
        int8_t source = 2;

        void format(serdes::packet &p) override
        {
            p.add(id);
            p.add(length, [&]() noexcept
                  { return length < 3; }); // a validation criteria
            p.add(source);
            p.pad(32);
        }
    };

    struct command : header_type
    {
        bool flags[3] = {true, false, true};
        double x = -1.0, y = -2.0, z = 3.14;
        uint32_t pattern = 0xABCD0123;

        void format(serdes::packet &p) override
        {
            header_type::format(p);
            p + serdes::pad<int>(5) + flags + serdes::bitpack<uint32_t, int>(pattern, 23) + serdes::align<int>(8) + x + y + z;
        }
    };

    struct compound_command : serdes::packet_base
    {
        command beginning_data = {};
        uint16_t ending_data[3] = {1, 2, 3};

        void format(serdes::packet &p) override
        {
            beginning_data.format(p);
            p + ending_data;
        }
    };

    // save the original struct data
    uint16_t serialized_data[40] = {};
    compound_command c;
    c.store(serialized_data);
    ASSERT_EQUALS(serialized_data, {0x0900, 0x0102, 0x0000, 0x0000, 0x059A, 0x0246, 0xBFF0, 0x0000, 0x0000,
                                    0x0000, 0xC000, 0x0000, 0x0000, 0x0000, 0x4009, 0x1EB8, 0x51EB, 0x851F,
                                    0x0001, 0x0002, 0x0003});
    {
        // clear out the struct with zeros
        uint16_t empty_data[40] = {};
        c.load(empty_data);
        ASSERT_EQUALS(c.beginning_data.id, 0);
        ASSERT_EQUALS(c.beginning_data.length, 0);
        ASSERT_EQUALS(c.beginning_data.source, 0);
        ASSERT_EQUALS(c.beginning_data.flags[0], false);
        ASSERT_EQUALS(c.beginning_data.flags[1], false);
        ASSERT_EQUALS(c.beginning_data.flags[2], false);
        ASSERT_EQUALS(c.beginning_data.x, 0.0);
        ASSERT_EQUALS(c.beginning_data.y, 0.0);
        ASSERT_EQUALS(c.beginning_data.z, 0.0);
        ASSERT_EQUALS(c.beginning_data.pattern, 0);
        ASSERT_EQUALS(c.ending_data, {0, 0, 0});
    }
    c.load(serialized_data);
    ASSERT_EQUALS(c.beginning_data.id, 9);
    ASSERT_EQUALS(c.beginning_data.length, 1);
    ASSERT_EQUALS(c.beginning_data.source, 2);
    ASSERT_EQUALS(c.beginning_data.flags[0], true);
    ASSERT_EQUALS(c.beginning_data.flags[1], false);
    ASSERT_EQUALS(c.beginning_data.flags[2], true);
    ASSERT_EQUALS(c.beginning_data.x, -1.0);
    ASSERT_EQUALS(c.beginning_data.y, -2.0);
    ASSERT_EQUALS(c.beginning_data.z, 3.14);
    ASSERT_EQUALS(c.beginning_data.pattern, 0x4D0123);
    ASSERT_EQUALS(c.ending_data, {1, 2, 3});
}

static void test_edittable_formats()
{
    struct coordinates : serdes::packet_base
    {
        int32_t x = -9, y = 10, z = -11;
        serdes::formatter edittable_format[3] = {
            serdes::init_this_formatter(x),
            serdes::init_this_formatter(y),
            serdes::init_this_formatter(z)};
        void format(serdes::packet &p) override
        {
            p + edittable_format;
        }
    };

    uint16_t serial_data[6] = {0x0000, 0x0001, 0x0000, 0x0002, 0xFFFF, 0xFFFB};
    coordinates A;
    A.load(serial_data);
    ASSERT_EQUALS(A.x, 1);
    ASSERT_EQUALS(A.y, 2);
    ASSERT_EQUALS(A.z, -5);
    std::fill(serial_data, serial_data + sizeof(serial_data) / sizeof(serial_data[0]), 0);
    uint8_t replacement_y = 0xE0;
    A.edittable_format[1] = serdes::init_formatter(replacement_y);
    A.x = 0xF345F012;
    A.y = 0xFADA9876; // wont be used (replaced above)
    A.z = 0xFEE35432;
    A.store(serial_data);
    ASSERT_EQUALS(serial_data, {0xF345, 0xF012, 0xE0FE, 0xE354, 0x3200, 0x0000});
    serial_data[2] = 0x97FE;
    A.load(serial_data);
    ASSERT_EQUALS(replacement_y, 0x97);
}

static void test_bitpacking_and_strings()
{
    uint32_t serial_data[10] = {};

    // serialize some data
    serdes::packet(serial_data) << 0xABCD_u16 << "hello!" << 123_i8 << serdes::bitpack<int32_t, int>(-9_i32, serdes::bit_length(6));

    ASSERT_EQUALS(serial_data, {0xABCD6865_u32, 0x6C6C6F21_u32, 0x007BDC00_u32, 0x00000000_u32});

    uint16_t x{};
    int8_t y{};
    int32_t z{};
    char str[7]{};

    // deserialize the data
    serdes::packet(serial_data) >> x >> str >> y >> serdes::bitpack<int32_t, int>(z, serdes::bit_length(6));

    ASSERT_EQUALS(x, 0xABCD);
    ASSERT_EQUALS(y, 123);
    ASSERT_EQUALS(z, -9);
    ASSERT_EQUALS(str, {'h', 'e', 'l', 'l', 'o', '!', '\0'});
}
static void test_alignment_and_padding()
{
    {
        uint16_t serial_data[10] = {};
        serdes::packet p(serial_data);
        p << serdes::pad<int>(10) << 0xFBCD_u16 << serdes::align<int>(32) << serdes::bitpack<int32_t, int>(0x12F, serdes::bit_length(4)) << 0x1ABC_u16;
        ASSERT_EQUALS(serial_data, {0x003E, 0xF340, 0xF1AB, 0xC000});

        uint16_t x;
        uint16_t y;
        uint16_t z;
        p >> serdes::pad<int>(10) >> x >> serdes::align<int>(32) >> serdes::bitpack<uint16_t, int>(y, serdes::bit_length(4)) >> z;

        ASSERT_EQUALS(x, 0xFBCD);
        ASSERT_EQUALS(y, 0xF);
        ASSERT_EQUALS(z, 0x1ABC);
    }
    {
        struct test_struct : serdes::packet_base
        {
            uint16_t x = 0xFBCD;
            uint16_t y = 0x012F;
            uint16_t z = 0x1ABC;
            void format(serdes::packet &p) override
            {
                p + serdes::pad<int>(10) + x + serdes::align<int>(32) + serdes::bitpack<uint16_t, int>(y, serdes::bit_length(4)) + z;
            }
        };
        test_struct object;
        uint32_t serial_data_u32[10] = {};
        object.store(serial_data_u32);
        ASSERT_EQUALS(serial_data_u32, {0x003EF340_u32, 0xF1ABC000_u32});
        uint8_t serial_data_u8[10] = {};
        object.store(serial_data_u8);
        ASSERT_EQUALS(serial_data_u8, {0x00, 0x3E, 0xF3, 0x40, 0xF1, 0xAB, 0xC0, 0x00});
        uint8_t zero_serial_data_u8[10] = {};
        object.load(zero_serial_data_u8);
        ASSERT_EQUALS(object.x, 0);
        ASSERT_EQUALS(object.y, 0);
        ASSERT_EQUALS(object.z, 0);
        object.load(serial_data_u32);
        ASSERT_EQUALS(object.x, 0xFBCD);
        ASSERT_EQUALS(object.y, 0xF);
        ASSERT_EQUALS(object.z, 0x1ABC);
        object.load(zero_serial_data_u8);
        object.load(serial_data_u8);
        ASSERT_EQUALS(object.x, 0xFBCD);
        ASSERT_EQUALS(object.y, 0xF);
        ASSERT_EQUALS(object.z, 0x1ABC);
    }
}
static void test_error_catching()
{
    struct coordinates : serdes::packet_base
    {
        int32_t x = -9, y = 10, z = -11;
        bool flags[8] = {};

        // defining the format to use for both serialization and deserialization
        void format(serdes::packet &p) override
        {
            auto y_with_check = serdes::init_formatter(y, [&]() noexcept
                                                       { return y > 6; });
            p + x + y_with_check + z + flags;
        }
    };

    {
        coordinates A;

        // providing some invalid data on purpose
        uint16_t serial_data[6] = {0x0000, 0x0001, 0x0000, 0x0002, 0xFFFF, 0xFFFB};

        // confirming the error is caught on load
        auto load_result = A.load(serial_data);
        ASSERT_EQUALS(static_cast<int>(load_result.status), static_cast<int>(serdes::status_e::INVALID_FIELD));
        ASSERT_EQUALS(load_result.bits, 64);

        // confirming the error is caught on store
        auto store_result = A.store(serial_data);
        ASSERT_EQUALS(static_cast<int>(store_result.status), static_cast<int>(serdes::status_e::INVALID_FIELD));
        ASSERT_EQUALS(store_result.bits, 32);

        // changing the value to be valid
        A.y = 100;

        // providing some data which is too small on purpose
        uint16_t serial_data_too_small[2] = {0x0000, 0x0001};

        // confirming the error is caught on store
        auto store_result2 = A.store(serial_data_too_small);
        ASSERT_EQUALS(static_cast<int>(store_result2.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));
        ASSERT_EQUALS(store_result2.bits, 32);

        // confirming that the format can be used with no errors
        uint16_t large_serial_data[7] = {0x0000, 0x0001, 0x0000, 0x00FF, 0xFFFF, 0xFFFB, 0xAAAA};
        auto store_result3 = A.load(large_serial_data);
        ASSERT_EQUALS(static_cast<int>(store_result3.status), static_cast<int>(serdes::status_e::NO_ERROR));
        ASSERT_EQUALS(store_result3.bits, 104);
        ASSERT_EQUALS(A.flags, {true, false, true, false, true, false, true, false});
    }
    {
        uint8_t serial_data[10] = {};
        auto load_status = (serdes::packet(serial_data) >> serdes::init_formatter(0xF01E_u16));
        ASSERT_EQUALS(static_cast<int>(load_status.status), static_cast<int>(serdes::status_e::NO_LOAD_TO_RVALUE));
        ASSERT_EQUALS(load_status.bit_offset, 0);
    }
    {
        // nullptr catches
        uint8_t *null_data = nullptr;
        uint16_t *null_serial_data = nullptr;

        int value = 1;
        auto load_status = (serdes::packet(null_data, 100) >> value);
        ASSERT_EQUALS(static_cast<int>(load_status.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));
        ASSERT_EQUALS(load_status.bit_offset, 0);

        auto store_status = (serdes::packet(null_data, 100) << value);
        ASSERT_EQUALS(static_cast<int>(store_status.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));
        ASSERT_EQUALS(store_status.bit_offset, 0);

        coordinates A;

        // confirming the error is caught on loads
        auto load_result = A.load(null_serial_data);
        ASSERT_EQUALS(static_cast<int>(load_result.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));
        ASSERT_EQUALS(load_result.bits, 0);

        load_result = A.load(null_serial_data, 100);
        ASSERT_EQUALS(static_cast<int>(load_result.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));
        ASSERT_EQUALS(load_result.bits, 0);

        // confirming the error is caught on stores
        auto store_result = A.store(null_serial_data);
        ASSERT_EQUALS(static_cast<int>(store_result.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));
        ASSERT_EQUALS(store_result.bits, 0);

        store_result = A.store(null_serial_data, 100);
        ASSERT_EQUALS(static_cast<int>(store_result.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));
        ASSERT_EQUALS(store_result.bits, 0);
    }
}

static void test_formatter_lambdas()
{
    uint16_t value = 0xDCBA;
    serdes::formatter x(serdes::init_formatter(value));
    serdes::formatter y(serdes::init_formatter(0xF01E_u16));
    uint16_t z_value = 0x9876;
    serdes::formatter z{[&](serdes::packet &p)
                        { p + z_value; }};
    uint8_t serial_data[10] = {};
    auto store_status = (serdes::packet(serial_data) << x << y << z);
    ASSERT_EQUALS(serial_data, {0xDC, 0xBA, 0xF0, 0x1E, 0x98, 0x76, 0x00});
    ASSERT_EQUALS(static_cast<int>(store_status.status), static_cast<int>(serdes::status_e::NO_ERROR));
    ASSERT_EQUALS(store_status.bit_offset, 48);
    auto load_status = (serdes::packet(serial_data) >> x >> y >> z);
    ASSERT_EQUALS(static_cast<int>(load_status.status), static_cast<int>(serdes::status_e::NO_LOAD_TO_RVALUE));
    ASSERT_EQUALS(load_status.bit_offset, 16);
}

static void test_delimited_arrays()
{
    struct my_delimited_data : serdes::packet_base
    {
        char data[100] = {};
        void format(serdes::packet &p) override
        {
            p + serdes::delimited_array<char>(data, '\0');
        }
    };

    const size_t serial_data_size = 50;
    uint8_t serial_data[serial_data_size] = "Hello World!";
    my_delimited_data object;

    auto load_result = object.load(serial_data);
    ASSERT_EQUALS(strcmp(object.data, "Hello World!"), 0);
    ASSERT_EQUALS(load_result.bits, 104);
    ASSERT_EQUALS(static_cast<int>(load_result.status), static_cast<int>(serdes::status_e::NO_ERROR));

    serdes::packet(serial_data) << "other stuff";
    auto load2_result = object.load(serial_data);
    ASSERT_EQUALS(strcmp(object.data, "other stuff"), 0);
    ASSERT_EQUALS(load2_result.bits, 96);
    ASSERT_EQUALS(static_cast<int>(load2_result.status), static_cast<int>(serdes::status_e::NO_ERROR));
    strcpy(object.data, "small");
    auto store_result = object.store(serial_data);
    ASSERT_EQUALS(strcmp(reinterpret_cast<char *>(&serial_data[0]), "small"), 0);
    ASSERT_EQUALS(store_result.bits, 6 * 8);
    ASSERT_EQUALS(static_cast<int>(store_result.status), static_cast<int>(serdes::status_e::NO_ERROR));

    for (size_t i = 0; i < sizeof(object.data) / sizeof(object.data[0]); i++)
        object.data[i] = 'h';
    auto store_result2 = object.store(serial_data);
    bool all_serial_data_matched = true;
    for (size_t i = 0; i < serial_data_size; i++)
        all_serial_data_matched &= serial_data[i] == 'h';
    ASSERT_EQUALS(all_serial_data_matched, true);
    ASSERT_EQUALS(store_result2.bits, 400);
    ASSERT_EQUALS(static_cast<int>(store_result2.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));

    uint8_t serial_data2[100] = {};
    auto store_result4 = object.store(serial_data2);
    ASSERT_EQUALS(store_result4.bits, 8 * 100);
    ASSERT_EQUALS(static_cast<int>(store_result4.status), static_cast<int>(serdes::status_e::DELIMITER_NOT_FOUND));
}

static void test_nested_delimited_arrays()
{
    struct my_bitpacked_char : serdes::packet_base
    {
        char data = 0;
        void format(serdes::packet &p) override
        {
            p + serdes::pad<int>(2) + serdes::bitpack<char, int>(data, 6);
        }
        my_bitpacked_char &operator=(const char x)
        {
            data = x;
            return *this;
        }
        bool operator==(const my_bitpacked_char x) const
        {
            return data == x.data;
        }
    };
    struct my_delimited_data : serdes::packet_base
    {
        my_bitpacked_char data[100] = {};
        void format(serdes::packet &p) override
        {
            p + serdes::delimited_array<my_bitpacked_char>(data, my_bitpacked_char());
        }
    };

    const size_t serial_data_size = 50;
    uint8_t serial_data[serial_data_size] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 0};
    my_delimited_data object;

    auto load_result = object.load(serial_data);
    for (size_t i = 1; i < 13; i++)
        ASSERT_EQUALS(object.data[i - 1].data, char(i));
    ASSERT_EQUALS(object.data[13].data, char(0));

    ASSERT_EQUALS(load_result.bits, 104_zu);
    ASSERT_EQUALS(static_cast<int>(load_result.status), static_cast<int>(serdes::status_e::NO_ERROR));

    char new_str[] = {8, 3, 6, 4, 5, 4, 7, 6, 5, 10, 2, 0};
    serdes::packet(serial_data) << new_str;
    auto load2_result = object.load(serial_data);
    for (size_t i = 0; i < sizeof(new_str) / sizeof(new_str[0]); i++)
        ASSERT_EQUALS(object.data[i].data, new_str[i]);
    ASSERT_EQUALS(load2_result.bits, 96);
    ASSERT_EQUALS(static_cast<int>(load2_result.status), static_cast<int>(serdes::status_e::NO_ERROR));
    object.data[2] = 12;
    object.data[4] = 15;
    object.data[5] = 0;
    auto store_result = object.store(serial_data);
    ASSERT_EQUALS(serial_data[2], uint8_t(12));
    ASSERT_EQUALS(serial_data[4], uint8_t(15));
    ASSERT_EQUALS(serial_data[5], uint8_t(0));
    ASSERT_EQUALS(store_result.bits, 6 * 8);
    ASSERT_EQUALS(static_cast<int>(store_result.status), static_cast<int>(serdes::status_e::NO_ERROR));

    for (size_t i = 0; i < sizeof(object.data) / sizeof(object.data[0]); i++)
        object.data[i] = 3;
    auto store_result2 = object.store(serial_data);
    bool all_serial_data_matched = true;
    for (size_t i = 0; i < serial_data_size; i++)
        all_serial_data_matched &= serial_data[i] == 3;
    ASSERT_EQUALS(all_serial_data_matched, true);
    ASSERT_EQUALS(store_result2.bits, 400);
    ASSERT_EQUALS(static_cast<int>(store_result2.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));

    uint8_t serial_data2[100] = {};
    auto store_result4 = object.store(serial_data2);
    ASSERT_EQUALS(store_result4.bits, 8 * 100);
    ASSERT_EQUALS(static_cast<int>(store_result4.status), static_cast<int>(serdes::status_e::DELIMITER_NOT_FOUND));
}

static void test_bitpacked_delimited_arrays()
{
    struct my_delimited_data : serdes::packet_base
    {
        unsigned char data[100] = {};
        void format(serdes::packet &p) override
        {
            p + serdes::bitpack<serdes::delimited_array<unsigned char>, int>(serdes::delimited_array<unsigned char>(data, '\0'), 4);
        }
    };

    const size_t serial_data_size = 25;
    uint8_t expected_data[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0xF, 0x1, 0x1, 0x2, 0x2, 0x3, 0x3, 0x4, 0x4, 0x5, 0x0};
    uint8_t serial_data[serial_data_size] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x50};
    my_delimited_data object;

    auto load_result = object.load(serial_data);
    for (size_t i = 0; i < sizeof(expected_data); i++)
        ASSERT_EQUALS(object.data[i], expected_data[i]);
    ASSERT_EQUALS(load_result.bits, 104_zu);
    ASSERT_EQUALS(static_cast<int>(load_result.status), static_cast<int>(serdes::status_e::NO_ERROR));

    unsigned char new_str[] = {0xFF, 0x11, 0x22, 0x33, 0x44, 0x51, 0x78, 0x9A, 0xBC, 0xDE, 0x56, 0x50};
    uint8_t expected_data2[] = {0xF, 0xF, 0x1, 0x1, 0x2, 0x2, 0x3, 0x3, 0x4, 0x4, 0x5, 0x1, 0x7, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0x5, 0x6, 0x5, 0x0};
    serdes::packet(serial_data) << new_str;
    auto load2_result = object.load(serial_data);
    for (size_t i = 0; i < sizeof(expected_data2); i++)
        ASSERT_EQUALS(object.data[i], expected_data2[i]);
    ASSERT_EQUALS(load2_result.bits, 96);
    ASSERT_EQUALS(static_cast<int>(load2_result.status), static_cast<int>(serdes::status_e::NO_ERROR));
    object.data[0] = 0xA;
    object.data[1] = 0xB;
    object.data[2] = 0xC;
    object.data[3] = 0xD;
    object.data[4] = 0xE;
    object.data[5] = 0x0;
    auto store_result = object.store(serial_data);
    ASSERT_EQUALS(serial_data[0], uint8_t(0xAB));
    ASSERT_EQUALS(serial_data[1], uint8_t(0xCD));
    ASSERT_EQUALS(serial_data[2], uint8_t(0xE0));
    ASSERT_EQUALS(store_result.bits, 6 * 4);
    ASSERT_EQUALS(static_cast<int>(store_result.status), static_cast<int>(serdes::status_e::NO_ERROR));

    for (size_t i = 0; i < sizeof(object.data) / sizeof(object.data[0]); i++)
        object.data[i] = 0x3;
    auto store_result2 = object.store(serial_data);
    bool all_serial_data_matched = true;
    for (size_t i = 0; i < serial_data_size; i++)
        all_serial_data_matched &= serial_data[i] == 0x33;
    ASSERT_EQUALS(all_serial_data_matched, true);
    ASSERT_EQUALS(store_result2.bits, 200);
    ASSERT_EQUALS(static_cast<int>(store_result2.status), static_cast<int>(serdes::status_e::EXCEEDED_SERIAL_SIZE));

    uint8_t serial_data2[50] = {};
    auto store_result4 = object.store(serial_data2);
    ASSERT_EQUALS(store_result4.bits, 8 * 50);
    ASSERT_EQUALS(static_cast<int>(store_result4.status), static_cast<int>(serdes::status_e::DELIMITER_NOT_FOUND));
}

static void test_virtual_formatters()
{
    struct optional_and_mandatory_data_test : serdes::packet_base
    {
        uint8_t x = 0;
        serdes::formatter optional_data = {serdes::virtual_formatter};
        uint8_t y = 0;
        serdes::formatter mandatory_data = {serdes::pure_virtual_formatter};

        void format(serdes::packet &packet) override
        {
            packet + x + optional_data + y + mandatory_data;
        }
    };
    optional_and_mandatory_data_test obj;
    uint8_t data[4] = {};
    auto store_result = obj.store(data);
    ASSERT_EQUALS(store_result.bits, 16_zu);
    ASSERT_EQUALS(static_cast<int>(store_result.status), static_cast<int>(serdes::status_e::FORMATTER_NOT_SET));

    auto load_result = obj.load(data);
    ASSERT_EQUALS(load_result.bits, 16_zu);
    ASSERT_EQUALS(static_cast<int>(load_result.status), static_cast<int>(serdes::status_e::FORMATTER_NOT_SET));
}

static void test_object_oriented_virtual_formatters()
{
    struct packet_format : serdes::packet_base
    {
        uint8_t id = 0;
        serdes::formatter payload = {serdes::pure_virtual_formatter};
        uint16_t checksum = 0;
        serdes::formatter optional_trailing_data = {serdes::virtual_formatter};

        void format(serdes::packet &p) override
        {
            p + id + payload + checksum + optional_trailing_data;
        }
    };

    struct amperage_command : packet_format
    {
        uint64_t amperage = 0;

        amperage_command()
        {
            packet_format::payload = {serdes::init_formatter(amperage)};
        }
    };

    struct voltage_command : packet_format
    {
        struct payload_t : serdes::packet_base
        {
            uint32_t voltage = 0;
            void format(serdes::packet &p) override
            {
                p + voltage;
            }
        } payload = {};

        voltage_command()
        {
            packet_format::payload = {serdes::init_formatter(payload)};
        }
    };

    uint8_t serial_data[] = {0xAB, 0x01, 0x02, 0x03, 0x04, 0xCD, 0xEF, 0x05, 0x06, 0x07, 0x08};

    amperage_command obj;
    auto result = obj.load(serial_data);
    ASSERT_EQUALS(static_cast<int>(result.status), static_cast<int>(serdes::status_e::NO_ERROR));
    ASSERT_EQUALS(obj.id, 0xAB_u8);
    ASSERT_EQUALS(obj.amperage, 0x01020304CDEF0506_u64);
    ASSERT_EQUALS(obj.checksum, 0x0708_u16);

    uint8_t serial_data2[11] = {};
    obj.store(serial_data2);
    ASSERT_EQUALS(serial_data2, {0xAB, 0x01, 0x02, 0x03, 0x04, 0xCD, 0xEF, 0x05, 0x06, 0x07, 0x08});

    voltage_command obj2;
    auto result2 = obj2.load(serial_data);
    ASSERT_EQUALS(static_cast<int>(result2.status), static_cast<int>(serdes::status_e::NO_ERROR));
    ASSERT_EQUALS(obj2.id, 0xAB_u8);
    ASSERT_EQUALS(obj2.payload.voltage, 0x01020304_u32);
    ASSERT_EQUALS(obj2.checksum, 0xCDEF_u16);
}

static void testset_serdes()
{
    test_variable_arrays();
    test_variable_packet_base_arrays();
    test_fixed_sized_arrays();
    test_bitpacked_arrays();
    test_dynamic_bitlength_captures();
    test_aligned_byte_arrays();
    test_inheritance_nesting();
    test_edittable_formats();
    test_bitpacking_and_strings();
    test_alignment_and_padding();
    test_error_catching();
    test_formatter_lambdas();
    test_delimited_arrays();
    test_nested_delimited_arrays();
    test_bitpacked_delimited_arrays();
    test_virtual_formatters();
    test_object_oriented_virtual_formatters();
}

#ifndef DISBALE_TESTS_MAIN
int main()
{
    testset_serdes();
    PRINT_SUMMARY();
}
#endif