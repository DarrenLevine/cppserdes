/// @example 06_variable_length_formats.cpp
/// @brief This example demonstrates how to define a variable sized array in a format

#include "../include/bitprint.h"
#include "../include/serdes.h"

struct my_info : serdes::packet_base
{
    uint8_t length = 0;
    uint8_t data[10] = {};
    uint8_t bit_flags = 0;

    void format(serdes::packet &p)
    {
        // here the number of elements in the "data" array changes based on the length field
        // as well as the number of bits in the "bit_flags" field
        p + length + serdes::array(data, length) + serdes::bitpack(bit_flags, length);
    }
};

int main()
{
    uint16_t serial_data[4] = {};
    { // storing some info according to the predefined format
        my_info object;
        object.length = 3;
        object.data[0] = 0xAB;
        object.data[1] = 0xCD;
        object.data[2] = 0xEF;
        object.bit_flags = 0xFF;
        object.store(serial_data);
        serdes::printhex(serial_data);
    }

    { // using the same format to load the data back into a new object
        my_info new_object;
        new_object.load(serial_data);
        printf("recovered data = [%u]{0x%X, 0x%X, 0x%X}\n\n",
               new_object.length, new_object.data[0], new_object.data[1], new_object.data[2]);

        printf("only %u flags recovered = ", new_object.length);
        serdes::printbin(new_object.bit_flags);
    }
}