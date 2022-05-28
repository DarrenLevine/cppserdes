#ifndef _TEST_UTILITIES_H_
#define _TEST_UTILITIES_H_

#include <algorithm>
#include <initializer_list>
#include "../include/bitprint.h"
#include "../include/serdes.h"
using namespace serdes::literals;

template <typename T, size_t N, typename R = T(&&)[N]>
inline R i_list(T(&&init_val)[N])
{
    return std::forward<R>(init_val);
}

long unsigned int tests = 0;
long unsigned int passes = 0;

template <typename T1, typename T2>
bool byte_cmp(T1 lhs, T2 rhs_init)
{
    ++tests;
    T1 rhs = rhs_init;
    for (size_t i = 0; i < sizeof(T1); i++)
        if (reinterpret_cast<uint8_t *>(&lhs)[i] != reinterpret_cast<uint8_t *>(&rhs)[i])
            return false;
    ++passes;
    return true;
}

template <typename B, typename D, bool bin_print = true, size_t N = 0, size_t N2 = 0>
bool byte_cmp(B (&result_array)[N], D (&expected_array)[N2])
{
    ++tests;
    if (N2 > N)
        return false;
    for (size_t i = 0; i < N2; i++)
        if (result_array[i] != expected_array[i])
            return false;
    ++passes;
    return true;
}

template <typename T1, typename T2, typename std::enable_if<!std::is_pointer<T2>::value && !std::is_array<typename std::remove_reference<T2>::type>::value, bool *>::type = nullptr>
inline void assert_equals_test(T1 lhs, T2 rhs, const char *test_statement, const char *va_args, const char *filename, int line_number)
{
    if (!byte_cmp(lhs, rhs))
    {
        printf("FAILED ASSERT_EQUALS(%s,%s) at %s:%i \n", test_statement, va_args, filename, line_number);
        printf(">>>    ");
        serdes::printhex(lhs, true);
        printf("    != ");
        serdes::printhex(rhs);
    }
}
template <typename B, typename D, size_t N = 0, size_t N2 = 0>
inline void assert_equals_test(B (&lhs)[N], D(&&rhs_init)[N2], const char *test_statement, const char *va_args, const char *filename, int line_number)
{
    B rhs[N2];
    for (size_t i = 0; i < N2; i++)
        rhs[i] = rhs_init[i];
    if (!byte_cmp(lhs, rhs))
    {
        printf("FAILED ASSERT_EQUALS(%s,%s) at %s:%i \n", test_statement, va_args, filename, line_number);
        printf(">>>    ");
        serdes::printhex(lhs, true);
        printf("    != ");
        serdes::printhex(rhs);
    }
}

#define ASSERT_EQUALS(lhs, ...) assert_equals_test(lhs, __VA_ARGS__, #lhs, #__VA_ARGS__, __FILE__, __LINE__)

#define PRINT_SUMMARY()                                                       \
    printf("\n--------------------------------\n%s (%lu/%lu tests passed)\n", \
           (tests == passes ? "PASSED!" : "FAILED!"), passes, tests)
#define PRINT_SUMMARY_AND_RETURN_EXIT_CODE() \
    PRINT_SUMMARY();                         \
    if (tests != passes)                     \
        return 1;                            \
    return 0

template <typename B, bool bin_print = true, size_t N = 0>
void test_cmp_arrays(B (&result_array)[N], B(&&expected_array)[N])
{
    bool passed = true;
    for (size_t i = 0; i < N && passed; i++)
        passed &= result_array[i] == expected_array[i];
    if (!passed)
    {
        printf("FAILED.\nexpected = ");
        if (bin_print)
            serdes::printbin(expected_array);
        else
            serdes::printhex(expected_array);
        printf("actual   = ");
        if (bin_print)
            serdes::printbin(result_array);
        else
            serdes::printhex(result_array);
        printf("\n");
    }
    ++tests;
    passes += passed;
}

template <typename B, bool bin_print = true, typename T = void, size_t N = 0>
void test_bitcpy_insert(int line_num, T inserted_value, size_t bits, size_t bit_offset, B(&&expected_array)[N], B init = ~B())
{
    B result_array[N];
    for (size_t i = 0; i < N; i++)
        result_array[i] = init;
    bool passed = true;
    serdes::bitcpy(result_array, inserted_value, bit_offset, bits);
    for (size_t i = 0; i < N && passed; i++)
        passed &= result_array[i] == expected_array[i];
    if (!passed)
    {
        printf("line %i FAILED bits=%zu bit_offset=%zu. inserted = ", line_num, bits, bit_offset);
        if (bin_print)
            serdes::printbin(inserted_value);
        else
            serdes::printhex(inserted_value);
        printf("expected = ");
        if (bin_print)
            serdes::printbin(expected_array);
        else
            serdes::printhex(expected_array);
        printf("actual   = ");
        if (bin_print)
            serdes::printbin(result_array);
        else
            serdes::printhex(result_array);
        printf("\n");
    }
    ++tests;
    passes += passed;
}

BITCPY_INT128_CONDITIONAL_DEFINE_C(
    inline __uint128_t form_uint128_t(uint64_t x, uint64_t y) {
        return (static_cast<__uint128_t>(x) << 64) | static_cast<__uint128_t>(y);
    });

#define run_type_tests(x)                                      \
    do                                                         \
    {                                                          \
        x<uint8_t>();                                          \
        x<uint16_t>();                                         \
        x<uint32_t>();                                         \
        x<uint64_t>();                                         \
        BITCPY_INT128_CONDITIONAL_DEFINE_C(x<__uint128_t>();); \
    } while (0)

#endif // _TEST_UTILITIES_H_
