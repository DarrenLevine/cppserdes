/// @example 01_simple_example.cpp
/// @brief This example demonstrates how to move data into a serial array, or out
/// of a serial array by defining a format on the fly. The formats support
/// bitpacking, alignment, padding, and any data type you'd like to serialize

#include "../include/serdes.h"
#include <stdio.h>

int main()
{
    // the serial data can be a: uint8_t[], uint16_t[], uint32_t[], or uint64_t[], without any changes to the other code
    uint32_t serial_data[10] = {};

    // serialize some data (pack the data into the serial_data array from left to right)
    serdes::packet(serial_data) << 0xABCD << "hello!" << 123 << serdes::bitpack(-9, serdes::bit_length(6));

    int x, y, z;
    char str[7];

    // deserialize the data (reads the data from serial_data array from left to right)
    serdes::packet(serial_data) >> x >> str >> y >> serdes::bitpack(z, serdes::bit_length(6));

    // will print out: "recovered data: {x = 0xABCD, str = hello!, y = 123, z = -9}"
    printf("recovered data: {x = 0x%X, str = \"%s\", y = %i, z = %i}\n\n", static_cast<unsigned int>(x), str, y, z);
}