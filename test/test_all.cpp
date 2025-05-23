//run using make
#define DISBALE_TESTS_MAIN
#include "test_bitcpy_to_array.cpp"
#include "test_bitcpy_from_array.cpp"
#include "test_serdes.cpp"
#include "test_custom_types.cpp"
#include "test_multiple_cpp_files.h"

// ensure no functions call new
void *operator new(size_t size)
{
    ASSERT_EQUALS(1, 2);
    printf("called new(%zu)!\n", size);
    return malloc(size);
}

int main()
{
    testset_to_array();
    testset_from_array();
    testset_custom_types();
    testset_serdes();
    ASSERT_EQUALS(exercise_separate_cpp_file(), 0xABCDEF0100020304_u64);

    PRINT_SUMMARY_AND_RETURN_EXIT_CODE();
}
