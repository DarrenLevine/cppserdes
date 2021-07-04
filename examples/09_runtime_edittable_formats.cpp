/// @example 09_runtime_edittable_formats.cpp
/// @brief This example demonstrates defining a list of serdes::formatter object
/// which can be used as an editable format. Fields and formats can be swapped
/// out flexibly, enabling use cases such as easily customizable telemetry messages.

#include "../include/serdes.h"
#include <stdio.h>
#include "../test/notify_when_dynamic_allocation_used.h"

struct coordinates : serdes::packet_base
{
    int32_t x = -9, y = 10, z = -11;

    // This packet's format is editable instead of hard coded, because it is stored
    // as data here. Read object_oriented_serial.cpp for the hard coded version
    // which will have faster performance, and use less memory, but the format
    // will not be editable.
    serdes::formatter edittable_format[3] = {
        serdes::init_formatter(x),
        serdes::init_formatter(serdes::bitpack(y, 16)),
        serdes::init_formatter(z)};

    // defining the format to use for both serialization and deserialization
    void format(serdes::packet &p) override
    {
        p + edittable_format;
    }
};

int main()
{
    // if you have some serial data
    uint16_t serial_data[6] = {0x0018, 0x0001, 0x0002, 0xFFFF, 0xFFFB, 0x0000};

    // and an object with a serdes::packet_base base
    coordinates obj;

    // you can convert from serial data to a packet_base object
    auto load_result = obj.load(serial_data);

    // edit the format to anything you'd like
    obj.edittable_format[0] = serdes::init_formatter(uint32_t(0xABCDEF01));

    // you can convert from a packet_base object to serial data
    auto store_result = obj.store(serial_data);

    // printing out some information about the process
    printf("Loaded x = %i, y = %i, z = %i (%zu bits total) with %s\n",
           obj.x, obj.y, obj.z, load_result.bits, serdes::status2str(load_result.status));
    printf("Stored x = %i, y = %i, z = %i (%zu bits total) with %s\n",
           obj.x, obj.y, obj.z, store_result.bits, serdes::status2str(store_result.status));
    printf("Replaced portion of the format = 0x%04X%04X\n", serial_data[0], serial_data[1]);
}