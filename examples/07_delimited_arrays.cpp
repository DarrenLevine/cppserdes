/// @example 07_delimited_arrays.cpp
/// @brief This example demonstrates a serializing and deserializing a dynamically
/// sized array by looking for a special reserved value marking the end of the
/// array, called a "delimiter". The typical use case is for null terminated
/// strings, though any type/value pair could be used.

#include "../include/serdes.h"
#include <stdio.h>

struct my_delimited_data : serdes::packet_base
{
    char data[100] = {};

    // defining the format to use for both serialization and deserialization
    void format(serdes::packet &p) override
    {
        p + serdes::delimited_array(data, '\0');
    }
};

int main()
{
    uint8_t serial_data[50] = "Hello World!";
    my_delimited_data object;

    auto load_result = object.load(serial_data);
    printf("Loaded \"%s\" (%zu bits total) with %s\n",
           object.data, load_result.bits, serdes::status2str(load_result.status));

    serdes::packet(serial_data) << "now we'll print out this instead";
    auto load2_result = object.load(serial_data);
    printf("Loaded \"%s\" (%zu bits total) with %s\n",
           object.data, load2_result.bits, serdes::status2str(load2_result.status));

    auto store_result = object.store(serial_data);
    printf("Stored \"%s\" (%zu bits total) with %s\n",
           serial_data, store_result.bits, serdes::status2str(store_result.status));
}