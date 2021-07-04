/// @example 04_object_oriented_serial.cpp
/// @brief This example demonstrates how a single format can be defined using a virtual
/// format method, which can be used for both serialization and deserialization.

#include "../include/serdes.h"
#include <stdio.h>

struct coordinates : serdes::packet_base
{
    int32_t x = -9, y = 10, z = -11;

    // defining the format to use for both serialization and deserialization
    void format(serdes::packet &serdes_process_object) override
    {
        // the format is defined as "x, y, z" using the + operator to separate fields
        serdes_process_object + x + y + z;

        // the .add() method can be used instead to accomplish the same thing:
        //  serdes_process_object.add(x);
        //  serdes_process_object.add(y);
        //  serdes_process_object.add(z);
    }
};

int main()
{
    // if you have some serial data
    uint16_t serial_data[6] = {0x0000, 0x0001, 0x0000, 0x0002, 0xFFFF, 0xFFFB};

    // and an object with a serdes::packet_base base
    coordinates obj;

    // you can convert from serial data to a packet_base object
    auto load_result = obj.load(serial_data);

    // and/or you can convert from a packet_base object to serial data
    auto store_result = obj.store(serial_data);

    // printing out some information about the process
    printf("Loaded x = %i, y = %i, z = %i (%zu bits total) with %s\n",
           obj.x, obj.y, obj.z, load_result.bits, serdes::status2str(load_result.status));
    printf("Stored x = %i, y = %i, z = %i (%zu bits total) with %s\n",
           obj.x, obj.y, obj.z, store_result.bits, serdes::status2str(store_result.status));
}