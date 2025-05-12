/// @example 11_using_atomics_with_bitcpy.cpp
/// @brief This example demonstrates how to serialize/deserialize data atomically
///
/// NOTE: Serialization/Deserialization will work out-of-the-box on any wrapper class
///       with a 'T load() const' and 'void store(T)' method, not just "std::atomic"
///       but also any custom classes. The load()/store() methods will be automatically
///       detected and called appropriately. This means that #include "serdes.h" does
///       not also pull in #include <atomic> or any other compatible types even though
///       they are still supported - you'll only get what you use.

#include "../include/serdes.h"
#include <stdio.h>
#include <atomic>

uint8_t original_value = 0xAB;
std::atomic<uint8_t> atomic_value{original_value};
static void test_recovered_original_value()
{
    if (atomic_value == original_value)
        printf("Atomic value recovered (0x%X).\n", atomic_value.load());
    else
        printf("...failed to recover atomic value (0x%X)!\n", atomic_value.load());
}

int main()
{
    uint8_t storage_array[3] = {};

    // store the value
    serdes::bitcpy(storage_array, atomic_value);

    // corrupt the value
    atomic_value = 0u;

    // recover the value with bitcpy
    serdes::bitcpy(atomic_value, storage_array);
    test_recovered_original_value();

    // recover the value with streams
    storage_array[0] = 0;
    serdes::packet(storage_array, 2) << atomic_value;
    atomic_value = 0x56u;
    serdes::packet(storage_array, 2) >> atomic_value;
    test_recovered_original_value();

    // recover the value with .add formatters
    storage_array[0] = 0;
    serdes::packet(storage_array, sizeof(storage_array), 0, serdes::mode_e::STORING) + atomic_value;
    atomic_value = 0x78u;
    serdes::packet(storage_array, sizeof(storage_array), 0, serdes::mode_e::LOADING) + atomic_value;
    test_recovered_original_value();
}