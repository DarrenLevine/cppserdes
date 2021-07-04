
#include "../include/serdes.h"
#include "test_multiple_cpp_files.h"

using namespace serdes::literals;

uint64_t exercise_separate_cpp_file()
{
    volatile uint64_t data[1] = {};
    serdes::packet(data) << 0xABCDEF01_u32 << 2_u16 << 3_u8 << 4_u8;
    return data[0];
}