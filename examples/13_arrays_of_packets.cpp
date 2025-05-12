/// @example 13_arrays_of_packets.cpp
/// @brief This example demonstrates how a complex format can be serialized in
/// a dynamically sized array

#include "../include/serdes.h"
#include <stdio.h>

struct coordinates final : serdes::packet_base
{
    uint8_t x = 0xAB, y = 0xCD, z = 0xEF;
    void format(serdes::packet &serdes_obj) final
    {
        serdes_obj + x + y + z;
    }
};

template <size_t capacity>
struct vector3d : serdes::packet_base
{
    uint16_t size = 0;
    coordinates values[capacity];

    void format(serdes::packet &serdes_obj) final
    {
        // this format records the size first, and then uses it
        // to decode/encode a max of "capacity" coordinate values
        serdes_obj + size + serdes::array(values, size);
    }
};

int main()
{
    // if you have some serial data
    uint16_t serial_data[] = {0x0003, 0x0102, 0x0304, 0x0506, 0x0708, 0x090A, 0x0B0C};

    // and an object with a serdes::packet_base base
    vector3d<100> obj;

    // you can convert from serial data to a packet_base object
    auto load_result = obj.load(serial_data);

    // printing out the results show that the appropriate array size was parsed and used
    printf("loaded vector3d<100> (with %s) = [%u/100]{\n",
           serdes::status2str(load_result.status), obj.size);
    for (size_t i = 0; i < obj.size; i++)
        printf("    xyz[%zu] = {0x%02X, 0x%02X, 0x%02X}\n",
               i, obj.values[i].x, obj.values[i].y, obj.values[i].z);
    printf("}\n");
}