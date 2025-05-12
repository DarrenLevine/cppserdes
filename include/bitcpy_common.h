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
#include <cstring>
#include "bitcpy_sized_pointer.h"

// use the compilation flag "-DconfigBITCPY_DISABLE_CONSTEXPR" to disable this
#ifndef CONSTEXPR_ABOVE_CPP11
#if defined(configBITCPY_DISABLE_CONSTEXPR) || !defined(__cplusplus) || (__cplusplus < 201402L)
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
#endif

// use the compilation flag "-DconfigBITCPY_DISABLE_CONSTEXPR_NON_LIT" to disable this
// NOTE: "Non-literal variables (and labels and gotos) in constexpr functions" not suppored by apple: https://en.cppreference.com/w/cpp/compiler_support#cpp14
#ifndef CONSTEXPR_ABOVE_CPP11_AND_NON_LITERAL_STORAGE
#if defined(configBITCPY_DISABLE_CONSTEXPR_NON_LIT) || !defined(__cplusplus) || (__cplusplus < 201402L) || defined(__APPLE__)
/// @brief if == 1 constexpr for bitcpy functions using non-literal storage is supported
#define BITCPY_CONSTEXPR_NON_LITERAL_STORAGE_SUPPORTED 0
/// @brief resolves automatically to "constexpr" if C++14 or greater
#define CONSTEXPR_ABOVE_CPP11_AND_NON_LITERAL_STORAGE
#else
/// @brief if == 1 constexpr for bitcpy functions using non-literal storage is supported
#define BITCPY_CONSTEXPR_NON_LITERAL_STORAGE_SUPPORTED 1
/// @brief resolves automatically to "constexpr" if C++14 or greater
#define CONSTEXPR_ABOVE_CPP11_AND_NON_LITERAL_STORAGE constexpr
#endif
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
        static constexpr float version = 1.1f;

        /// @brief CppSerdes library #define major version
        #define LIB_CPP_SERDES_VERSION_MAJOR 1

        /// @brief CppSerdes library #define minor version
        #define LIB_CPP_SERDES_VERSION_MINOR 1

        /// @brief CppSerdes library create single version number, from two major and
        /// minor numbers, suitable for >= comparisons.
        #define LIB_CPP_SERDES_VERSION_MAJOR_MINOR(major, minor)  (major*1000 + minor)

        /// @brief CppSerdes library single version number, suitable for >= comparisons.
        #define LIB_CPP_SERDES_VERSION LIB_CPP_SERDES_VERSION_MAJOR_MINOR(LIB_CPP_SERDES_VERSION_MAJOR, LIB_CPP_SERDES_VERSION_MINOR)

        // Example of how to check version using a #if:
        //      #if LIB_CPP_SERDES_VERSION >= LIB_CPP_SERDES_VERSION_MAJOR_MINOR(1, 1)
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

    // implementation details
    namespace detail
    {
        // always returns false if included - this is just used to highlight type name in
        // static_assert failure message to make debugging easier
        template <typename UnsupportedType>
        constexpr bool type_is_supported() { return false; }
    }

    struct packet;

    /// @brief wrapper construct used to let the user provide custom (de)serialization support
    /// for types without modifying the original type.
    template <typename UnsupportedType>
    struct custom_type
    {
        // used to detect if a type has a custom override (if this method is removed it means it has an override)
        // so you must not specify this method name in any custom type override.
        void has_no_override__DO_NOT_SPECIFY_THIS(){}

        // optional default bit size needed for custom non-compointtypes, where sizeof(T)*8 won't
        // accurately determine the serialized bit size. If the default_bits setting isn't specified,
        // and the "size_t bits" argument in the format method below isn't specified, then bitpacking
        // will not be supported by the custom type.
        static constexpr size_t default_bits = 0;

        // a overloadable definition for how to format some item into/out-of a serial packet,
        // this default method will emit a compiler error if it's ever used, indicating a type
        // has no definition for how it should be formatted into/out-of a serial packet, and one should be added.
        template <typename UnsupportedTypeSpecialized>
        static void format(packet &pkt, UnsupportedTypeSpecialized &&item, size_t bits)
        {
            static_assert(
                detail::type_is_supported<UnsupportedType>(),
                "\n"
                "--------------------------------------------------------------------------------------------------\n"
                "UnsupportedType has no format definition (or bitpacking was used on a custom type with no bitpacking\n"
                " support)!\n"
                "Please add serialization support via one of the four options:\n"
                "    1) Add 'T load() const' and 'void store(T val)' methods to your type, if it's a type wrapper such\n"
                "          as 'std::atomic<T>', CppSerdes will use the load/store during serializing/deserializing.\n"
                "    2) Add a 'void format(serdes::packet&)' method to your type if you're allowed to modify the type\n"
                "           and want to keep the serialization format information co-located with the type.\n"
                "    3) Inherit from 'serdes::packet_base' AND add a 'void format(serdes::packet&)' method.\n"
                "          This has the additional benefit compared to option #2 of adding extra\n"
                "          load, store, and stream (de)serialization operator methods. Note that there is no\n"
                "          memory downside to using packet_base - it is compatible with 'Empty Base Optimization'.\n"
                "    4) Provide a custom_type<T> specialized definition for types you cannot modify. For example:\n"
                "               template<> struct serdes::custom_type<YOUR_CUSTOM_TYPE_NAME> {\n"
                "                   template <typename T>\n"
                "                   static void format(serdes::packet &pkt, T &&item) {\n"
                "                       pkt + item.custom_stuff; // <-- example\n"
                "                   }\n"
                "               }\n"
                "        NOTE: If you want your type to be able to support bit-packing (rare), you need this instead:\n"
                "               template<> struct serdes::custom_type<YOUR_CUSTOM_TYPE_NAME> {\n"
                "                   static constexpr size_t default_bits = 7; // used when no bits are specified\n"
                "                   template <typename T>\n"
                "                   static void format(serdes::packet &pkt, T &&item, size_t bits) {\n"
                "                       pkt + serdes::bitpack(item.custom_stuff, bits); // <-- example\n"
                "                   }\n"
                "               }\n"
                "--------------------------------------------------------------------------------------------------\n");
        }
    };

    // implementation details
    namespace detail
    {

#if BITCPY_CONSTEXPR_SUPPORTED
        /// @brief returns true if running on a little endian platform
        constexpr bool on_little_endian_platform()
#else
        /// @brief returns true if running on a little endian platform
        inline bool on_little_endian_platform()
#endif
        {
            const uint16_t num = 1u;
            return *reinterpret_cast<const uint8_t *>(&num) == 1u;
        }

        /// @brief same as std::remove_cvref, but compatible with C++11
        template<class T>
        struct remove_cvref_cpp11
        {
            using type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;
        };

        /// @brief checks if the type is a built-in type
        template <typename T>
        using is_built_in_type =
            std::integral_constant<bool,
                    std::is_pointer<typename remove_cvref_cpp11<T>::type>::value ||
                    std::is_arithmetic<typename remove_cvref_cpp11<T>::type>::value ||
                    std::is_integral<typename remove_cvref_cpp11<T>::type>::value ||
                    std::is_enum<typename remove_cvref_cpp11<T>::type>::value>;

        /// @brief same as std::void_t but included here for C++11 support
        template<typename...> using void_t_if_valid = void;

        /// @brief detecting classes that have a "void format(serdes::packet&)" method
        template <typename, typename = void>
        struct has_format_method : std::false_type
        {
        };
        template <typename T>
        struct has_format_method<
            T,
            void_t_if_valid<
                decltype(std::declval<T>().format(std::declval<packet&>()))>>
            : std::true_type
        {
        };

        /// @brief detecting classes that have any "serdes::custom_type<T>" definitions
        template <typename, typename = void>
        struct has_custom_type_override : std::true_type
        {
        };
        template <typename T>
        struct has_custom_type_override<
            T,
            void_t_if_valid<
                decltype(std::declval<custom_type<T>>().has_no_override__DO_NOT_SPECIFY_THIS())>>
            : std::false_type
        {
        };

        /// @brief detecting classes that have a "serdes::custom_type<T>" definition that includes bitpacking support
        template <typename, typename = void>
        struct has_custom_type_with_bit_support : std::false_type
        {
        };
        template <typename T>
        struct has_custom_type_with_bit_support<
            T,
            void_t_if_valid<
                decltype(std::declval<custom_type<T>>().format(std::declval<packet&>(), std::declval<T>(), std::declval<size_t>()))>>
            : std::true_type
        {
        };

        /// @brief detecting classes that have a "serdes::custom_type<T>" definition that does NOT include bitpacking support
        template <typename, typename = void>
        struct has_custom_type_without_bit_support : std::false_type
        {
        };
        template <typename T>
        struct has_custom_type_without_bit_support<
            T,
            void_t_if_valid<
                decltype(std::declval<custom_type<T>>().format(std::declval<packet&>(), std::declval<T>()))>>
            : std::true_type
        {
        };

        /// @brief extracting the inner wrapped type of classes, such as "std::atomic<inner_type>"
        template <typename T>
        struct get_inner_wrapped_type { };
        template <template <typename> class T_wrapper, typename T>
        struct get_inner_wrapped_type<T_wrapper<T>>
        {
            using type = T;
        };

        /// @brief detecting classes that have 'T load() const' and 'void store(T)' methods (with any inner type)
        template <typename, typename = void>
        struct has_load_and_store : std::false_type
        {
        };
        template <typename T, template <typename> class T_wrapper>
        struct has_load_and_store<
            T_wrapper<T>,
            void_t_if_valid<
                decltype(std::declval<T_wrapper<T>>().store(std::declval<T>())),
                decltype(std::declval<const T_wrapper<T>>().load()),
                typename std::enable_if<!has_format_method<T_wrapper<T>>::value>::type>>
            : std::true_type
        {
        };

        /// @brief detecting classes that have 'T load() const' and 'void store(T)' methods where the inner type is a builtin type
        template <typename, typename = void>
        struct has_load_and_store_of_builtin : std::false_type
        {
        };
        template <typename T, template <typename> class T_wrapper>
        struct has_load_and_store_of_builtin<
            T_wrapper<T>,
            void_t_if_valid<
                decltype(std::declval<T_wrapper<T>>().store(std::declval<T>())),
                decltype(std::declval<const T_wrapper<T>>().load()),
                typename std::enable_if<
                    !has_format_method<T_wrapper<T>>::value &&
                    is_built_in_type<T>::value
                    >::type>>
            : std::true_type
        {
        };

        /// @brief detecting classes that have 'T* data()' and 'size_t size()' methods
        template <typename T, typename = void>
        struct has_data_and_size : std::false_type
        {
            using elem_type = T;
        };
        template <typename T>
        struct has_data_and_size<
            T,
            void_t_if_valid<
                decltype(std::declval<typename remove_cvref_cpp11<T>::type>().data()),
                decltype(std::declval<typename remove_cvref_cpp11<T>::type>().size()),
                typename std::enable_if<!has_format_method<typename remove_cvref_cpp11<T>::type>::value>::type>>
            : std::true_type
        {
            using elem_type = typename std::remove_reference<decltype(*std::declval<typename std::remove_reference<T>::type>().data())>::type;
        };

        /// @brief checks to see if a type is supported by the bitcpy functional API
        template <typename T>
        using supported_by_bitcpy =
            std::integral_constant<bool,
                is_built_in_type<T>::value ||
                has_load_and_store_of_builtin<typename remove_cvref_cpp11<T>::type>::value>;

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
                !has_load_and_store_of_builtin<T>::value &&
                !is_sized_pointer<T>::value,
            T>::type;
        template <typename T>
        using requires_load_and_store_of_builtin = typename std::enable_if<
                has_load_and_store_of_builtin<T>::value,
            T>::type;
        template <typename T>
        using requires_data_and_size = typename std::enable_if<
                has_data_and_size<T>::value,
            T>::type;
        template <typename T>
        using requires_pointer_type = typename std::enable_if<
            std::is_pointer<T>::value,
            T>::type;
        template <typename T>
        using requires_not_pointer_or_load_store = typename std::enable_if<
            !std::is_pointer<T>::value &&
            !has_load_and_store_of_builtin<T>::value,
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

/// @brief If configCPP_SERDES_ALLOW_REINTERPRET_BUILTINS_UB is defined, cppserdes will allows UB (undefined behaviour)
/// allowing reinterpret case of built-in types which may speed up performance on some systems by preventing an extra
/// std::memcpy - allowing in-place data manipulation.
/// Note that no memory alignment concerns are needed using this optimization since the reinterpreted memory is always
/// the same size and in the same location as the original memory. Please be very careful with applying this optimization,
/// and confirm that it works with your specific compiler/platform/optimization-level.
///    N3337 [basic.lval]/10: Is UB if a type that is the signed or unsigned type corresponding to a cv-qualified version
///                           of the dynamic type of the object.
#ifdef configCPP_SERDES_ALLOW_REINTERPRET_BUILTINS_UB
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
#endif // #ifdef configCPP_SERDES_ALLOW_REINTERPRET_BUILTINS_UB

        template <typename T>
        __attribute__((pure)) constexpr T bitmask(const size_t onecount) noexcept
        {
            using Tunsigned = typename std::make_unsigned<T>::type;
            return static_cast<T>((static_cast<Tunsigned>(~static_cast<Tunsigned>(0u)) >> ((sizeof(Tunsigned) * 8u) - onecount)) * static_cast<Tunsigned>(onecount != 0));
        }
        BITCPY_INT128_CONDITIONAL_DEFINE_C(
            template <>
            __attribute__((pure)) constexpr __int128_t bitmask<__int128_t>(const size_t onecount) noexcept {
                using Tunsigned = __uint128_t;
                return static_cast<Tunsigned>(-(onecount != 0)) & (static_cast<Tunsigned>(-1) >> ((sizeof(Tunsigned) * 8u) - onecount));
            } template <>
            __attribute__((pure)) constexpr __uint128_t bitmask<__uint128_t>(const size_t onecount) noexcept {
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
        template <typename T_val, template <typename> class T_wrap>
        struct default_bitsize<T_wrap<T_val>>
        {
            static constexpr size_t value =
                has_load_and_store<T_wrap<T_val>>::value
                    ? default_bitsize<T_val>::value
                    : sizeof(T_wrap<T_val>) * 8u;
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
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const uint8_t *const data) {
                using T2 = uint8_t;
                return (static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(data)) << 64) | static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(&data[8 / sizeof(T2)]));
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const uint16_t *const data) {
                using T2 = uint16_t;
                return (static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(data)) << 64) | static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(&data[8 / sizeof(T2)]));
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const uint32_t *const data) {
                using T2 = uint32_t;
                return (static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(data)) << 64) | static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(&data[8 / sizeof(T2)]));
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const uint64_t *const data) {
                using T2 = uint64_t;
                return (static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(data)) << 64) | static_cast<__uint128_t>(big_endian_memcpy<uint64_t, T2>(&data[8 / sizeof(T2)]));
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline __uint128_t big_endian_memcpy(const __uint128_t *const data) {
                return data[0];
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline uint64_t big_endian_memcpy(const __uint128_t *const data) {
                return static_cast<uint64_t>(data[0] >> 64);
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline uint32_t big_endian_memcpy(const __uint128_t *const data) {
                return static_cast<uint32_t>(data[0] >> 96);
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline uint16_t big_endian_memcpy(const __uint128_t *const data) {
                return static_cast<uint16_t>(data[0] >> 112);
            }

            template <>
            CONSTEXPR_ABOVE_CPP11 inline uint8_t big_endian_memcpy(const __uint128_t *const data) {
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
    } // namespace detail
}
#endif // _BITCPY_COMMON_H_
