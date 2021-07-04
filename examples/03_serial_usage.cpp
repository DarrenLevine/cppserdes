/// @example 03_serial_usage.cpp
/// @brief These examples demonstrate using the serdes::packet class with stream
/// operators as well as method operators, along with several format customization
/// techniques such as modifying padding, alignment, and and bit size (bitpacking)

#include "../include/serdes.h"
#include "../include/bitprint.h"

int main()
{
    constexpr size_t buffer_size = sizeof(int) * 3 / sizeof(uint32_t);

    // serializing and deserializing using the << >> stream operators
    {
        uint32_t buffer[buffer_size] = {};
        serdes::packet buffer_stream(buffer);

        // store some values into the buffer
        buffer_stream << 1 << 5 << 9;

        // display the stored serial data
        printf("stored data:    ");
        serdes::printhex(buffer);

        // recover the values from the buffer
        int x, y, z;
        buffer_stream >> x >> y >> z;

        printf("recovered data: {x = %i, y = %i, z = %i}\n\n", x, y, z);
    }

    // serializing and deserializing using the store/load/pad/align methods for bitpacking
    {
        uint32_t buffer[buffer_size] = {};
        serdes::packet buffer_stream(buffer);

        // store some values into the buffer
        buffer_stream.store(-5);
        buffer_stream.pad(1);       // adding 1 padding bit
        buffer_stream.store(-2, 3); // bitpacking into 3 bits
        buffer_stream.align(32);    // adding pad bits to allign to 32bit boundaries
        buffer_stream.store(-123);

        // display the stored serial data
        printf("stored data:    ");
        serdes::printhex(buffer);

        // recover the values from the buffer
        int x, y, z;
        buffer_stream.load(x);
        buffer_stream.pad(1);     // adding 1 padding bit
        buffer_stream.load(y, 3); // bitpacking into 3 bits
        buffer_stream.align(32);  // adding pad bits to allign to 32bit boundaries
        buffer_stream.load(z);

        printf("recovered data: {x = %i, y = %i, z = %i}\n\n", x, y, z);
    }

    // serializing and deserializing using the "<<" ">>" operators (additionally showing off how bitpacking can be applied to arrays)
    {
        uint32_t buffer[buffer_size] = {};
        serdes::packet buffer_stream(buffer);
        uint64_t flags[8] = {1, 0, 1, 0, 1, 0, 1, 1};

        // store some values into the buffer
        buffer_stream << -5 << serdes::pad(1) << serdes::bitpack(-2, 3) << serdes::align(16) << -123 << serdes::bitpack(flags, 1);

        // display the stored serial data
        printf("stored data:    ");
        serdes::printhex(buffer);

        // recover the values from the buffer
        int x, y, z;
        buffer_stream >> x >> serdes::pad(1) >> serdes::bitpack(y, 3) >> serdes::align(16) >> z >> serdes::bitpack(flags, 1);

        printf("recovered data: {x = %i, y = %i, z = %i}\n\n", x, y, z);
    }
}
