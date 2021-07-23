/// @file bitcpy_common.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief Defines common bitcpy details as well as a info::version number, and bit_length function
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _BITCPY_COMMON_H_
#define _BITCPY_COMMON_H_

#include <stdint.h>
#include <cstddef>
#include <type_traits>
#include <utility>
#include "bitcpy_sized_pointer.h"
#if !defined(configBITCPY_COMMON_DISABLE_ATOMIC_FWD_DECL) && !defined(_GLIBCXX_ATOMIC) && !defined(atomic)
namespace std
{
    template <typename _Tp>
    struct atomic; // forward declaration
}
#endif
#if !defined(__cplusplus) || (__cplusplus < 201402L)
/// @brief if == 1 constexpr for bitcpy is supported
#define BITCPY_CONSTEXPR_SUPPORTED 0
/// @brief resolves automatically to "constexpr" if C++14 or greater
#define CONSTEXPR_ABOVE_CPP11
#else
/// @brief if == 1 constexpr for bitcpy is supported
#define BITCPY_CONSTEXPR_SUPPORTED 1
/// @brief resolves automatically to "constexpr" if C++14 or greater
#define CONSTEXPR_ABOVE_CPP11 constexpr
#endif

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

    /// @brief CppSerdes library information
    struct info
    {
        /// @brief CppSerdes library version number
        static constexpr float version = 1.0;
    };

    /// @brief Does nothing but annotate a passed value as a bit length, used for writting clearer code.
    /// @param    x: bit length value
    /// @tparam   T: the type of the bit length value
    /// @return   constexpr size_t: the same bit length value that was used as the input
    template <typename T>
    constexpr T bit_length(T &&x) noexcept
    {
        return std::forward<T>(x);
    }

    // implimentation details
    namespace detail
    {
        template <typename>
        struct is_std_atomic : std::false_type
        {
        };
        template <typename T>
        struct is_std_atomic<std::atomic<T>> : std::true_type
        {
        };
        template <typename T>
        using requires_unsigned_type = typename std::enable_if<
            BITCPY_INT128_CONDITIONAL_DEFINE(
                std::is_same<T, __uint128_t>::value ||)(!std::is_same<T, char>::value && std::is_integral<T>::value && !std::is_same<T, bool>::value && !std::is_signed<T>::value) ||
                (std::is_same<T, char>::value && std::is_same<char, uint8_t>::value), // char is a special case, reinterpret it to uint8_t to reduce the template code generated if != uint8_t
            T>::type;
        template <typename T>
        using requires_bool_type = typename std::enable_if<
            std::is_same<T, bool>::value,
            T>::type;
        template <typename T>
        using requires_non_bool_type = typename std::enable_if<
            !std::is_same<T, bool>::value,
            T>::type;
        template <typename T>
        using requires_signed_type = typename std::enable_if<
            BITCPY_INT128_CONDITIONAL_DEFINE(std::is_same<T, __int128_t>::value ||)(!std::is_same<T, char>::value && std::is_integral<T>::value && !std::is_same<T, bool>::value && std::is_signed<T>::value) ||
                (std::is_same<T, char>::value && !std::is_same<char, uint8_t>::value), // char is a special case, reinterpret it to uint8_t to reduce the template code generated if != uint8_t
            T>::type;
        template <typename T>
        using requires_large_non_integral_type = typename std::enable_if<
            (sizeof(T) > 8) &&
                BITCPY_INT128_CONDITIONAL_DEFINE(!std::is_same<T, __uint128_t>::value &&
                                                 !std::is_same<T, __int128_t>::value &&) !is_sized_pointer<T>::value,
            T>::type;
        template <typename T>
        using requires_small_non_integral_type = typename std::enable_if<
            (sizeof(T) <= 8) &&
                BITCPY_INT128_CONDITIONAL_DEFINE(!std::is_same<T, __uint128_t>::value &&
                                                 !std::is_same<T, __int128_t>::value &&) !std::is_integral<T>::value &&
                !std::is_same<T, bool>::value &&
                !is_std_atomic<T>::value &&
                !is_sized_pointer<T>::value,
            T>::type;
        template <typename T>
        using requires_pointer_type = typename std::enable_if<
            std::is_pointer<T>::value,
            T>::type;
        template <typename T>
        using requires_not_a_pointer_type = typename std::enable_if<
            !std::is_pointer<T>::value,
            T>::type;

        template <size_t N>
        struct unsigned_type_sizeof
        {
        };
        template <>
        struct unsigned_type_sizeof<1>
        {
            using type = uint8_t;
        };
        template <>
        struct unsigned_type_sizeof<2>
        {
            using type = uint16_t;
        };
        template <>
        struct unsigned_type_sizeof<4>
        {
            using type = uint32_t;
        };
        template <>
        struct unsigned_type_sizeof<8>
        {
            using type = uint64_t;
        };
        BITCPY_INT128_CONDITIONAL_DEFINE_C(
            template <>
            struct unsigned_type_sizeof<16> {
                using type = __uint128_t;
            };);
        template <typename T>
        __attribute__((const)) const typename unsigned_type_sizeof<sizeof(T)>::type &reinterpret_as_unsigned(const T &value) noexcept
        {
            return *reinterpret_cast<const typename unsigned_type_sizeof<sizeof(T)>::type *>(&value);
        }
        template <typename T>
        __attribute__((const)) const volatile typename unsigned_type_sizeof<sizeof(T)>::type &reinterpret_as_unsigned(const volatile T &value) noexcept
        {
            return *reinterpret_cast<const volatile typename unsigned_type_sizeof<sizeof(T)>::type *>(&value);
        }
        template <typename T>
        __attribute__((const)) volatile typename unsigned_type_sizeof<sizeof(T)>::type &reinterpret_as_unsigned(volatile T &value) noexcept
        {
            return *reinterpret_cast<volatile typename unsigned_type_sizeof<sizeof(T)>::type *>(&value);
        }
        template <typename T>
        __attribute__((const)) typename unsigned_type_sizeof<sizeof(T)>::type &reinterpret_as_unsigned(T &value) noexcept
        {
            return *reinterpret_cast<typename unsigned_type_sizeof<sizeof(T)>::type *>(&value);
        }
        template <typename T>
        __attribute__((pure)) constexpr T bitmask(const size_t onecount) noexcept
        {
            using Tunsigned = typename std::make_unsigned<T>::type;
            return static_cast<T>((static_cast<Tunsigned>(~static_cast<Tunsigned>(0u)) >> ((sizeof(Tunsigned) * 8u) - onecount)) * static_cast<Tunsigned>(onecount != 0));
        }
        BITCPY_INT128_CONDITIONAL_DEFINE_C(
            template <>
            __attribute__((pure)) constexpr __int128_t bitmask<__int128_t>(const size_t onecount) noexcept
            {
                using Tunsigned = __uint128_t;
                return static_cast<Tunsigned>(-(onecount != 0)) & (static_cast<Tunsigned>(-1) >> ((sizeof(Tunsigned) * 8u) - onecount));
            } template <>
            __attribute__((pure)) constexpr __uint128_t bitmask<__uint128_t>(const size_t onecount) noexcept
            {
                using Tunsigned = __uint128_t;
                return static_cast<Tunsigned>(-(onecount != 0)) & (static_cast<Tunsigned>(-1) >> ((sizeof(Tunsigned) * 8u) - onecount));
            });
        template <typename T, requires_signed_type<T> * = nullptr>
        CONSTEXPR_ABOVE_CPP11 void extend_sign(T &x, const size_t bits) noexcept
        {
            if (bits >= sizeof(T) * 8u || bits == 0u)
                return;
            const T m = static_cast<T>(1u) << (bits - 1u);
            // faster than if-branch checking for the sign bit
            x = ((x & bitmask<T>(bits)) ^ m) - m;
        }
        template <typename T>
        struct default_bitsize
        {
            static constexpr size_t value = sizeof(T) * 8u;
        };
        template <>
        struct default_bitsize<bool>
        {
            static constexpr size_t value = 1u;
        };

        template <typename T, typename T2>
        CONSTEXPR_ABOVE_CPP11 inline T big_endian_memcpy(const T2 *const)
        {
            static_assert(sizeof(T) == 0,
                          "Use big_endian_memcpy_unsigned instead. Either you tried to use a "
                          "signed type which is not supported, or a type larger than 128bits");
        }

        //
        // uint8_t[] section
        //

        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint8_t big_endian_memcpy(const uint8_t *const data)
        {
            return data[0];
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint16_t big_endian_memcpy(const uint8_t *const data)
        {
            return static_cast<uint16_t>(data[0]) << 8 | static_cast<uint16_t>(data[1]);
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint32_t big_endian_memcpy(const uint8_t *const data)
        {
            return (static_cast<uint32_t>(data[0]) << 24) |
                   (static_cast<uint32_t>(data[1]) << 16) |
                   (static_cast<uint32_t>(data[2]) << 8) |
                   static_cast<uint16_t>(data[3]);
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint64_t big_endian_memcpy(const uint8_t *const data)
        {
            return (static_cast<uint64_t>(data[0]) << 56) |
                   (static_cast<uint64_t>(data[1]) << 48) |
                   (static_cast<uint64_t>(data[2]) << 40) |
                   (static_cast<uint64_t>(data[3]) << 32) |
                   (static_cast<uint64_t>(data[4]) << 24) |
                   (static_cast<uint64_t>(data[5]) << 16) |
                   (static_cast<uint64_t>(data[6]) << 8) |
                   static_cast<uint64_t>(data[7]);
        }

        //
        // uint16_t[] section
        //

        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint8_t big_endian_memcpy(const uint16_t *const data)
        {
            return static_cast<uint8_t>(data[0] >> 8);
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint16_t big_endian_memcpy(const uint16_t *const data)
        {
            return data[0];
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint32_t big_endian_memcpy(const uint16_t *const data)
        {
            return (static_cast<uint32_t>(data[0]) << 16) | static_cast<uint32_t>(data[1]);
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint64_t big_endian_memcpy(const uint16_t *const data)
        {
            return (static_cast<uint64_t>(data[0]) << 48) |
                   (static_cast<uint64_t>(data[1]) << 32) |
                   (static_cast<uint64_t>(data[2]) << 16) |
                   static_cast<uint64_t>(data[3]);
        }

        //
        // uint32_t[] section
        //

        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint8_t big_endian_memcpy(const uint32_t *const data)
        {
            return static_cast<uint8_t>(data[0] >> 24);
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint16_t big_endian_memcpy(const uint32_t *const data)
        {
            return static_cast<uint16_t>(data[0] >> 16);
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint32_t big_endian_memcpy(const uint32_t *const data)
        {
            return data[0];
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint64_t big_endian_memcpy(const uint32_t *const data)
        {
            return (static_cast<uint64_t>(data[0]) << 32) |
                   static_cast<uint64_t>(data[1]);
        }

        //
        // uint64_t[] section
        //

        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint8_t big_endian_memcpy(const uint64_t *const data)
        {
            return static_cast<uint8_t>(data[0] >> 56);
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint16_t big_endian_memcpy(const uint64_t *const data)
        {
            return static_cast<uint16_t>(data[0] >> 48);
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint32_t big_endian_memcpy(const uint64_t *const data)
        {
            return static_cast<uint32_t>(data[0] >> 32);
        }
        template <>
        CONSTEXPR_ABOVE_CPP11 inline uint64_t big_endian_memcpy(const uint64_t *const data)
        {
            return data[0];
        }

        BITCPY_INT128_CONDITIONAL_DEFINE_C(

            template <>
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const uint8_t *const data)
            {
                using T2 = uint8_t;
                return (static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(data)) << 64) | static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(&data[8 / sizeof(T2)]));
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const uint16_t *const data)
            {
                using T2 = uint16_t;
                return (static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(data)) << 64) | static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(&data[8 / sizeof(T2)]));
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const uint32_t *const data)
            {
                using T2 = uint32_t;
                return (static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(data)) << 64) | static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(&data[8 / sizeof(T2)]));
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const uint64_t *const data)
            {
                using T2 = uint64_t;
                return (static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(data)) << 64) | static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(&data[8 / sizeof(T2)]));
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const __uint128_t *const data)
            {
                return data[0];
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline uint64_t big_endian_memcpy(const __uint128_t *const data)
            {
                return static_cast<uint64_t>(data[0] >> 64);
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline uint32_t big_endian_memcpy(const __uint128_t *const data)
            {
                return static_cast<uint32_t>(data[0] >> 96);
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline uint16_t big_endian_memcpy(const __uint128_t *const data)
            {
                return static_cast<uint16_t>(data[0] >> 112);
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline uint8_t big_endian_memcpy(const __uint128_t *const data)
            {
                return static_cast<uint8_t>(data[0] >> 120);
            }

        );

        template <typename T, typename T2>
        CONSTEXPR_ABOVE_CPP11 inline T big_endian_memcpy_unsigned(const T2 *const v)
        {
            return big_endian_memcpy<
                typename detail::unsigned_type_sizeof<sizeof(T)>::type,
                typename detail::unsigned_type_sizeof<sizeof(T2)>::type>(v);
        }
    }
}
#endif // _BITCPY_COMMON_H_
