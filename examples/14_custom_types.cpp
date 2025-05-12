/// @example 14_custom_types.cpp
/// @brief This example demonstrates how to add serialization support to new and existing types

#include "../include/bitprint.h"
#include "../include/serdes.h"
#include <array>
#include <mutex>
#include <stdio.h>
#include <string>

// You can add serialization support via one of the four options:
//     1) Add 'T load() const' and 'void store(T val)' methods to your type, if it's a type wrapper such
//           as 'std::atomic<T>', CppSerdes will use the load/store during serializing/deserializing.
//     2) Add a 'void format(serdes::packet&)' method to your type if you're allowed to modify the type
//            and want to keep the serialization format information co-located with the type.
//     3) Inherit from 'serdes::packet_base' AND add a 'void format(serdes::packet&)' method.
//           This has the additional benefit compared to option #2 of adding extra
//           load, store, and stream (de)serialization operator methods. Note that there is no
//           memory downside to using packet_base - it is compatible with 'Empty Base Optimization'.
//     4) Provide a custom_type<T> specialized definition for types you cannot modify. For example:
//                template<> struct serdes::custom_type<YOUR_CUSTOM_TYPE_NAME> {
//                    template <typename T>
//                    static void format(serdes::packet &pkt, T &&item) {
//                        pkt + item.custom_stuff; // <-- example
//                    }
//                }
//         NOTE: If you want your type to be able to support bit-packing (rare), you need this instead:
//                template<> struct serdes::custom_type<YOUR_CUSTOM_TYPE_NAME> {
//                    static constexpr size_t default_bits = 7; // used when no bits are specified
//                    template <typename T>
//                    static void format(serdes::packet &pkt, T &&item, size_t bits) {
//                        pkt + serdes::bitpack(item.custom_stuff, bits); // <-- example
//                    }
//                }

//
// Example of OPTION #1: mutex_safe_wrapper with load/store methods
//
template <typename T>
class mutex_safe_wrapper
{
public:
    template <typename... T2>
    mutex_safe_wrapper(T2 &&...val) : value(std::forward<T2>(val)...) {}
    T load() const
    {
        std::lock_guard<std::mutex> lk(m);
        printf("mutex_safe_wrapper::load()\n");
        return value;
    }
    template <typename T2>
    void store(T2 &&val)
    {
        std::lock_guard<std::mutex> lk(m);
        printf("mutex_safe_wrapper::store(val)\n");
        value = std::forward<T2>(val);
    }

private:
    T value{};
    mutable std::mutex m{};
};

//
// Example of OPTION #2: coordinates_type (only adds a "void format(packet &)" method)
//
struct coordinates_type {
    uint8_t bits_per_coordinate{};
    mutex_safe_wrapper<int64_t> x{}, y{}, z{};
    void format(serdes::packet &serial_data) {
        // describes a 8 bit field conveying how many bits are in each
        // of the next three bit-packed x, y, z fields
        serial_data +
            bits_per_coordinate +
            serdes::bitpack(x, bits_per_coordinate) +
            serdes::bitpack(y, bits_per_coordinate) +
            serdes::bitpack(z, bits_per_coordinate);
    }
};

//
// Example of OPTION #3: coordinate_list (inherits from "packet_base" and has "void format(packet &)")
//
struct coordinate_list : serdes::packet_base {

    uint16_t num_coordinates{};
    std::array<coordinates_type, 100u> coordinates{};
    uint16_t crc16{};

    void format(serdes::packet &serial_data) {
        // describes a 16 bit length, followed by an array of
        // coordinate objects of that length
        serial_data +
            num_coordinates +
            serdes::array(coordinates, num_coordinates);

        // calculating a checksum and then using it to validate a crc16 field
        auto calculated = serial_data.calculate_crc<CRC16::CCITT_FALSE>(&crc16);
        serial_data + serdes::validate(crc16, [&] { return crc16 == calculated; });

        // print out the values if the crc was invalid
        if (serial_data.status == serdes::status_e::INVALID_FIELD)
            printf("Got an invalid crc! Got 0x%04X, calculated 0x%04X\n", crc16, calculated);
    }
};

//
// Example of OPTION #4: custom_type<std::string>
//
// In this example support is added for a std::string as a null delimited array with some hard upper limit.
// you could also do a size prefixed format, such as: "pkt + size + serdes::array(string, size);"
// or some other schema, the design decision for schema is entirely left up to you. CppSerdes provides
// enough tools to implement/design your own data transfer protocols without forcing you into a specific one.
template <>
struct serdes::custom_type<std::string> {
    template <typename T>
    static void format(serdes::packet &pkt, T &&item) {
        if (pkt.mode == serdes::mode_e::LOADING) {
            if constexpr (std::is_const_v<std::remove_reference_t<T>>)
                pkt.status = serdes::status_e::NO_LOAD_TO_RVALUE;
            else {
                item.reserve(2048u);
                pkt >> serdes::delimited_array(item.data(), '\00', 2048u);
            }
        }
        else
            pkt << serdes::delimited_array(item, '\00');
    }
};

//
// Demonstration of usage:
//
int main()
{
    // demonstrating method #4 with std::string works:
    std::array<uint8_t, 100> serial_data{};

    // storing string into some data array
    serdes::packet(serial_data) << std::string("Hello World!");
    
    // printing out what was stored
    printf("stored '%s' into serial data\n", serial_data.data());

    // loading serial data into a std::string object
    std::string some_string = "this will be overwritten";
    serdes::packet(serial_data) >> some_string;

    // printing out what was loaded
    printf("loaded '%s' into a std::string\n", some_string.c_str());

    // showing how options 1-3 can be used (and even with different serial array type sizes (uint8/16/32_t[] arrays),
    // without explicitly writing any support code for that in our custom types)
    coordinate_list complex_obj;
    uint8_t uint8_data[100] = {
        0x00, 0x03, 0x08, 0x11, 0x22, 0x33, 0x08, 0xAA, 0xBB, 0xCC, 0x08, 0xEE, 0xFF, 0xEF, 0x57, 0x19};

    auto stat = complex_obj.load(uint8_data);
    printf("loaded uint8_t[] %zu bits total with %s\n", stat.bits, serdes::status2str(stat.status));

    stat = complex_obj.store(uint8_data);
    printf("stored %zu bits total with %s\n", stat.bits, serdes::status2str(stat.status));

    uint16_t uint16_data[100] = {0x0003, 0x0811, 0x2233, 0x08AA, 0xBBCC, 0x08EE, 0xFFEF, 0x5719};
    stat = complex_obj.load(uint16_data);
    printf("loaded uint16_t[] %zu bits total with %s\n", stat.bits, serdes::status2str(stat.status));

    stat = complex_obj.store(uint16_data);
    printf("stored %zu bits total with %s\n", stat.bits, serdes::status2str(stat.status));

    uint32_t uint32_data[100] = {0x00030811, 0x223308AA, 0xBBCC08EE, 0xFFEF5719};
    stat = complex_obj.load(uint32_data);
    printf("loaded uint32_t[] %zu bits total with %s\n", stat.bits, serdes::status2str(stat.status));

    stat = complex_obj.store(uint32_data);
    printf("stored %zu bits total with %s\n", stat.bits, serdes::status2str(stat.status));
}