/// @example 12_compile_time_bitcpy_evaluation.cpp
/// @brief This example demonstrates serdes::bitcpy() being used purely during compile time

#include "../include/bitcpy.h"
#include "../include/bitliterals.h"
#include <stdio.h>

#if !BITCPY_CONSTEXPR_SUPPORTED
int main()
{
    printf("You can't use the constexpr bitcpy feature without C++14 or greater.\n"
           "Note: All other features will still work.\n");
}
#else
using namespace serdes::literals;

template <size_t N>
constexpr uint32_t decode_for_example(uint8_t(&&data)[N])
{
    uint32_t value = 0;
    serdes::bitcpy(value, data, 4, 24);
    return value;
}

template <uint32_t V>
struct force_compilation
{
    static constexpr uint32_t value = V;
};

int main()
{
    auto compile_time_data = force_compilation<decode_for_example({1_u8, 2_u8, 3_u8, 4_u8, 5_u8})>::value;
    if (compile_time_data == 0x102030)
    {
        printf("Evaluated at compile time!\n");
        return 0;
    }

    printf("Not evaluted at compile time!\n");
    return -1;
}
#endif