/// @example 08_error_handling.cpp
/// @brief This example demonstrates how several different errors can be caught
/// durring runtime. Both user defined errors, and serdes related errors, like
/// overflows.

#include "../include/serdes.h"
#include "../include/bitprint.h"
#include "../test/notify_when_dynamic_allocation_used.h"

struct coordinates : serdes::packet_base
{
    int32_t x = -9, y = 10, z = -11;
    bool flags[8] = {};

    // defining the format to use for both serialization and deserialization
    void format(serdes::packet &p) override
    {
        p.add(x);
        p.add(y, [&]() noexcept
              { return y > 6; }); // an arbitrary validity check function is attached to "y"

        if (p.status == serdes::status_e::INVALID_FIELD)
        {
            // you can check for any type of error in the format process
        }

        p.add(z);
        p.add(flags);

        // you can also define the validity criteria inline using the operator (+, >>, <<) syntax like this:
        //  p + x + serdes::init_formatter(y, [&]() noexcept { return y > 6; }) + z + flags;
    }
};

int main()
{
    coordinates obj;

    // providing some invalid data on purpose
    uint16_t serial_data[6] = {0x0000, 0x0001, 0x0000, 0x0002, 0xFFFF, 0xFFFB};

    // confirming the error is caught on load
    auto load_result = obj.load(serial_data);
    printf("%s at bit %zu during load process\n", serdes::status2str(load_result.status), load_result.bits);

    // confirming the error is caught on store
    auto store_result = obj.store(serial_data);
    printf("%s at bit %zu during store process\n", serdes::status2str(store_result.status), store_result.bits);

    // changing the value to be valid
    obj.y = 100;

    // providing some data which is too small on purpose
    uint16_t serial_data_too_small[2] = {0x0000, 0x0001};

    // confirming the error is caught on store
    auto store_result2 = obj.store(serial_data_too_small);
    printf("%s at bit %zu during store process\n", serdes::status2str(store_result2.status), store_result2.bits);

    // confirming that the format can be used with no errors
    uint16_t large_serial_data[7] = {0x0000, 0x0001, 0x0000, 0x00FF, 0xFFFF, 0xFFFB, 0xAAAA};
    auto store_result3 = obj.load(large_serial_data);
    printf("%s at bit %zu during load process\n", serdes::status2str(store_result3.status), store_result3.bits);

    // this should print out alternating boolean flags from the interpreted 0xAAAA value:
    //    {0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00}
    serdes::printhex(obj.flags);
}