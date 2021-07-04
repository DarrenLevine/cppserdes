/// @example 11_using_atomics_with_bitcpy.cpp
/// @brief This example demonstrates how to serialize/deserialize data atomically

#include "../include/bitcpy.h"
#include <stdio.h>
#include <atomic>

uint8_t original_value = 0xAB;
std::atomic<uint8_t> atomic_value{original_value};

int main()
{
    uint8_t storage_array[3] = {};

    // store the value
    serdes::bitcpy(storage_array, atomic_value);

    // corrupt the value
    atomic_value = 0u;

    // recover the value
    serdes::bitcpy(atomic_value, storage_array);

    if (atomic_value == original_value)
        printf("Atomic value recovered (0x%X).\n", atomic_value.load());
    else
        printf("...failed to recover atomic value (0x%X)!\n", atomic_value.load());
}