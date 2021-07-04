/// @example 02_bitcpy_usage.cpp
/// @brief This example demonstrates the low level serdes::bitcpy() function. The function
/// copies bits to/from serial arrays and variables (serialization/deserialization)
///
/// bitcpy follows a pattern similar to memcpy. For de-serialization use:
///
///     size_t bitcpy(T1 &dest, const T2 source[], const size_t bit_offset, const size_t bits)
///
/// and for serialization use:
///
///     size_t bitcpy(T1 dest[], const T2 source, const size_t bit_offset, const size_t bits)

#include "../include/bitcpy.h"
#include "../include/bitprint.h"

int main()
{
    constexpr size_t buffer_size = sizeof(int) * 3 / sizeof(uint32_t);

    // serializing and deserializing using bitcpy
    {
        uint32_t buffer[buffer_size] = {};

        // store some values into the buffer
        size_t bit_offset = 0;
        serdes::bitcpy(buffer, 1, bit_offset);
        bit_offset += 32;
        serdes::bitcpy(buffer, 5, bit_offset);
        bit_offset += 32;
        serdes::bitcpy(buffer, 6, bit_offset);

        // display the stored serial data
        printf("stored data:    ");
        serdes::printhex(buffer);

        // recover the values from the buffer
        int x, y, z;
        bit_offset = 0;
        serdes::bitcpy(x, buffer, bit_offset);
        bit_offset += 32;
        serdes::bitcpy(y, buffer, bit_offset);
        bit_offset += 32;
        serdes::bitcpy(z, buffer, bit_offset);

        printf("recovered data: {x = %i, y = %i, z = %i}\n\n", x, y, z);
    }

    // serializing and deserializing using with bitpacking/padding
    {
        uint32_t buffer[buffer_size] = {};

        // store some values into the buffer
        size_t bit_offset = 0;
        serdes::bitcpy(buffer, -5, bit_offset, serdes::bit_length(32));
        bit_offset += 33;
        serdes::bitcpy(buffer, -2, bit_offset, serdes::bit_length(3));
        bit_offset += 30;
        serdes::bitcpy(buffer, -123, bit_offset, serdes::bit_length(33));

        // display the stored serial data
        printf("stored data:    ");
        serdes::printhex(buffer);

        // recover the values from the buffer
        int x, y, z;
        bit_offset = 0;
        serdes::bitcpy(x, buffer, bit_offset, serdes::bit_length(32));
        bit_offset += 33;
        serdes::bitcpy(y, buffer, bit_offset, serdes::bit_length(3));
        bit_offset += 30;
        serdes::bitcpy(z, buffer, bit_offset, serdes::bit_length(33));

        printf("recovered data: {x = %i, y = %i, z = %i}\n\n", x, y, z);
    }
}