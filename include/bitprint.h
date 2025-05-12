/// @file bitprint.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief defines printhex() and printbin() funtions which print arrays or values in hexidecimal or binary
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _BITPRINT_H_
#define _BITPRINT_H_

#include <stdint.h>
#include <cstddef>
#include <stdio.h>
#include <type_traits>
#include <inttypes.h>

#ifdef __SIZEOF_INT128__
/// @brief automatically adds or removes __uint128_t and __int128_t support depending on if the compiler supports it
#define BITCPY_INT128_CONDITIONAL_DEFINE(...) __VA_ARGS__
/// @brief automatically adds or removes __uint128_t and __int128_t support depending on if the compiler supports it (with colon)
#define BITCPY_INT128_CONDITIONAL_DEFINE_C(...) __VA_ARGS__ static_assert(true, "")
#else
/// @brief automatically adds or removes __uint128_t and __int128_t support depending on if the compiler supports it
#define BITCPY_INT128_CONDITIONAL_DEFINE(...)
/// @brief automatically adds or removes __uint128_t and __int128_t support depending on if the compiler supports it (with colon)
#define BITCPY_INT128_CONDITIONAL_DEFINE_C(...) static_assert(true, "")
#endif

/// @brief CppSerdes library namespace
namespace serdes
{
    namespace bitprint_detail
    {
        template <typename T>
        struct make_signed
        {
            using type = typename std::make_signed<T>::type;
        };
        BITCPY_INT128_CONDITIONAL_DEFINE_C(
            template <>
            struct make_signed<__uint128_t> {
                using type = __int128_t;
            };);
        template <typename T>
        struct make_unsigned
        {
            using type = typename std::make_unsigned<T>::type;
        };
        BITCPY_INT128_CONDITIONAL_DEFINE_C(
            template <>
            struct make_unsigned<__uint128_t> {
                using type = __uint128_t;
            };
            template <>
            struct make_unsigned<__int128_t> {
                using type = __uint128_t;
            };);
        template <>
        struct make_unsigned<float>
        {
            using type = uint32_t;
        };
        template <>
        struct make_unsigned<double>
        {
            using type = uint64_t;
        };
        template <bool use_brackets>
        void print_bracket_newline(const bool add_newline)
        {
            if (use_brackets)
                printf((add_newline ? "}\n" : "}"));
            else if (add_newline)
                printf("\n");
        }
    }

    /// @brief prints uint8_t value in hex format
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    inline void printhex(const uint8_t data, const bool add_newline = true)
    {
        if (add_newline)
            printf("0x%02" PRIX8 "\n", data);
        else
            printf("0x%02" PRIX8, data);
    }

    /// @brief prints uint16_t value in hex format
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    inline void printhex(const uint16_t data, const bool add_newline = true)
    {
        if (add_newline)
            printf("0x%04" PRIX16 "\n", data);
        else
            printf("0x%04" PRIX16, data);
    }

    /// @brief prints uint32_t value in hex format
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    inline void printhex(const uint32_t data, const bool add_newline = true)
    {
        if (add_newline)
            printf("0x%08" PRIX32 "\n", data);
        else
            printf("0x%08" PRIX32, data);
    }

    /// @brief prints uint64_t value in hex format
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    inline void printhex(const uint64_t data, const bool add_newline = true)
    {
        if (add_newline)
            printf("0x%08" PRIX32 "%08" PRIX32 "\n",
                   static_cast<uint32_t>(data >> 32),
                   static_cast<uint32_t>(data));
        else
            printf("0x%08" PRIX32 "%08" PRIX32,
                   static_cast<uint32_t>(data >> 32),
                   static_cast<uint32_t>(data));
    }
    BITCPY_INT128_CONDITIONAL_DEFINE_C(

        /// prints __uint128_t value in hex format
        inline void printhex(const __uint128_t data, const bool add_newline = true)
        {
            if (add_newline)
                printf("0x%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "\n",
                       static_cast<uint32_t>(data >> 32 * 3),
                       static_cast<uint32_t>(data >> 32 * 2),
                       static_cast<uint32_t>(data >> 32 * 1),
                       static_cast<uint32_t>(data));
            else
                printf("0x%08" PRIX32 "%08" PRIX32 "%08" PRIX32 "%08" PRIX32,
                       static_cast<uint32_t>(data >> 32 * 3),
                       static_cast<uint32_t>(data >> 32 * 2),
                       static_cast<uint32_t>(data >> 32 * 1),
                       static_cast<uint32_t>(data));
        });

    /// @brief prints bool value in hex format
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    inline void printhex(const bool data, const bool add_newline = true)
    {
        printhex(uint8_t(data), add_newline);
    }

    /// @brief prints signed value in hex format
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    template <typename T = void,
              typename std::enable_if<
                  std::is_signed<T>::value
                      BITCPY_INT128_CONDITIONAL_DEFINE(|| std::is_same<T, __int128_t>::value),
                  T>::type * = nullptr>
    void printhex(const T data, const bool add_newline = true)
    {
        typedef typename bitprint_detail::make_unsigned<T>::type data_as_unsigned_type;
        union
        {
            T original_value;
            data_as_unsigned_type unsigned_value;
        } punned_data;
        punned_data.original_value = data;
        printhex(punned_data.unsigned_value, add_newline);
    }

    /// @brief prints an array in hex format
    /// @tparam   use_brackets: if true, curly brackets will surround the printed statement
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    template <bool use_brackets = true, typename T = void, size_t N = 0u>
    void printhex(const T (&data)[N], const bool add_newline = true)
    {
        if (use_brackets)
            printf("{");
        for (size_t i = 0; i < N; i++)
        {
            printhex(data[i], false);
            printf("%s", (i + 1u < N ? ", " : ""));
        }
        bitprint_detail::print_bracket_newline<use_brackets>(add_newline);
    }

    /// @brief prints an array in hex format
    /// @tparam   use_brackets: if true, curly brackets will surround the printed statement
    /// @param    data: c-array to print
    /// @param    size: number of elements to print
    /// @param    add_newline: if true, a newline will be added to the end
    template <bool use_brackets = true, typename T = void, size_t N = 0u>
    void printhex(const T (&data)[N], size_t size, const bool add_newline = true)
    {
        if (use_brackets)
            printf("{");
        if (size > N)
            size = N;
        for (size_t i = 0; i < size; i++)
        {
            printhex(data[i], false);
            printf("%s", (i + 1u < size ? ", " : ""));
        }
        bitprint_detail::print_bracket_newline<use_brackets>(add_newline);
    }

    /// @brief prints an array in binary format
    /// @tparam   use_brackets: if true, curly brackets will surround the printed statement
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    template <bool use_brackets = true, typename T = void, size_t N = 0u>
    void printbin(const T (&data)[N], const bool add_newline = true)
    {
        if (use_brackets)
            printf("{");
        for (size_t i = 0; i < N; i++)
        {
            for (size_t ii = 0; ii < sizeof(T); ii++)
            {
                const uint8_t data_element = static_cast<uint8_t>(
                    (data[i] >> (sizeof(T) * 8 - (ii + 1) * 8)) & 0xFFu);
                printf("%c%c%c%c%c%c%c%c",
                       ((data_element & 0x80u) ? '1' : '0'),
                       ((data_element & 0x40u) ? '1' : '0'),
                       ((data_element & 0x20u) ? '1' : '0'),
                       ((data_element & 0x10u) ? '1' : '0'),
                       ((data_element & 0x08u) ? '1' : '0'),
                       ((data_element & 0x04u) ? '1' : '0'),
                       ((data_element & 0x02u) ? '1' : '0'),
                       ((data_element & 0x01u) ? '1' : '0'));
            }
            printf("%s", (i + 1u < N ? ", " : ""));
        }
        bitprint_detail::print_bracket_newline<use_brackets>(add_newline);
    }

    /// @brief prints a non-array non-bool value in binary format
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    template <typename T, typename std::enable_if<!std::is_array<T>::value && !std::is_same<T, bool>::value, T>::type * = nullptr>
    void printbin(const T data, const bool add_newline = true)
    {
        typedef const T data_as_array_type[1];
        printbin<false>(*reinterpret_cast<data_as_array_type *>(&data), add_newline);
    }

    /// @brief prints a bool value in binary format
    /// @param    data: value to print
    /// @param    add_newline: if true, a newline will be added to the end
    inline void printbin(const bool data, const bool add_newline = true)
    {
        printbin(uint8_t(data), add_newline);
    }
}

#endif // _BITPRINT_H_
