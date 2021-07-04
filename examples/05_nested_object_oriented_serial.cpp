/// @example 05_nested_object_oriented_serial.cpp
/// @brief This example demonstrates nesting format definitions in an object
/// oriented manner. Both through inheritance, and through instance nesting.

#include "../include/serdes.h"
#include <stdio.h>
#include "../test/notify_when_dynamic_allocation_used.h"

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

// nesting via inheritance
struct obj_with_header_inherited : header_type
{
    bool flags[3] = {true, false, true};
    double x = -1.0, y = -2.0, z = 3.14;
    uint32_t pattern = 0xABCD0123;

    void format(serdes::packet &p) override
    {
        // inherited formats need to be called before or after any surrounding formats like so:
        header_type::format(p);

        p + serdes::pad(5) + flags + serdes::bitpack(pattern, 23) + serdes::align(8) + x + y + z;
    }
};

// nesting via nested instantiation of a obj_with_header_inherited type object
struct compound_type : serdes::packet_base
{
    obj_with_header_inherited beginning_data = {};
    uint16_t ending_data = 0x1234;

    void format(serdes::packet &p) override
    {
        // since the format is literally nested instead of inherited, it can be added to the format directly
        p + beginning_data + ending_data;
    }
};

int main()
{
    uint8_t reservoir[70] = {};

    compound_type object;
    auto result = object.store(reservoir);

    printf("Bits Stored  = %zu with %s\n", result.bits, serdes::status2str(result.status));

    // corrupting the data, so that it's obvious when its recovered
    object.beginning_data.id = 0;
    object.beginning_data.length = 0;
    object.beginning_data.source = 0;
    object.beginning_data.flags[0] = false;
    object.beginning_data.flags[1] = false;
    object.beginning_data.flags[2] = false;
    object.beginning_data.x = 0.0;
    object.beginning_data.y = 0.0;
    object.beginning_data.z = 0.0;
    object.beginning_data.pattern = 0;
    object.ending_data = 0;

    result = object.load(reservoir);

    printf("Bits Loaded  = %zu with %s\n", result.bits, serdes::status2str(result.status));

    printf("obj_with_header_inherited = {\n");
    printf("    id = %u\n", object.beginning_data.id);
    printf("    length = %u\n", object.beginning_data.length);
    printf("    source = %u\n", object.beginning_data.source);
    printf("    flags[0] = %u\n", object.beginning_data.flags[0]);
    printf("    flags[1] = %u\n", object.beginning_data.flags[1]);
    printf("    flags[2] = %u\n", object.beginning_data.flags[2]);
    printf("    x = %.2f\n", object.beginning_data.x);
    printf("    y = %.2f\n", object.beginning_data.y);
    printf("    z = %.2f\n", object.beginning_data.z);
    printf("    pattern = 0x%08X\n", object.beginning_data.pattern);
    printf("    ending_data = 0x%04X\n}", object.ending_data);
}