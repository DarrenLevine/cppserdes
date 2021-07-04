/// @file serdes_format_modifiers.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief Defines format modification classes: align, pad, bitpack, array, delimited_array
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _SERDES_FORMAT_MODIFIERS_H_
#define _SERDES_FORMAT_MODIFIERS_H_

#include "bitcpy_common.h"

/// @brief CppSerdes library namespace
namespace serdes
{
    /// @brief specifies a number of bits to be used to align the bit offset as a multiple of
    struct align
    {
        /// bit alignment reference value
        const size_t &value;

        /// @brief Construct a new align object, binding to a reference of the number of bits to align by
        /// @param    bits
        align(const size_t &bits) noexcept : value{bits} {}

        /// @brief Construct a new align object, binding to a rvalue of the number of bits to align by
        /// @param    bits
        align(size_t &&bits) noexcept : value{std::move(bits)} {}
    };

    /// @brief specifies a number of bits to be used to pad (add to) the current bit offset
    struct pad
    {
        /// bit pad reference value
        const size_t &value;

        /// @brief Construct a new pad object, binding to a reference of the number of bits to pad by
        /// @param    bits
        pad(const size_t &bits) noexcept : value{bits} {}

        /// @brief Construct a new pad object, binding to a rvalue of the number of bits to pad by
        /// @param    bits
        pad(size_t &&bits) noexcept : value{std::move(bits)} {}
    };

    /// @brief bitpack a value into exact the specified bits. If applied to an array
    /// it will be applied to each element of the array, not the total bits in the array
    /// @tparam   T: the type of the value bitpacked
    template <typename T>
    struct bitpack
    {
        /// @brief a reference to the bitpacked value
        T &value;

        /// @brief a reference to the number of bits to use when storing/loading the referenced value
        const size_t &bits;

        /// @brief Construct a new bitpack object from a rvalue value, and rvalue bit length
        /// @param    v: referenced value
        /// @param    b: number of bits to pack into
        bitpack(T &&v, size_t &&b) noexcept : value{v}, bits{b} {}

        /// @brief Construct a new bitpack object from a reference value, and rvalue bit length
        /// @param    v: referenced value
        /// @param    b: number of bits to pack into
        bitpack(T &v, size_t &&b) noexcept : value{v}, bits{b} {}

        /// @brief Construct a new bitpack object from a reference value, and reference bit length
        /// @param    v: referenced value
        /// @param    b: number of bits to pack into
        bitpack(T &v, const size_t &b) noexcept : value{v}, bits{b} {}

        /// @brief Construct a new bitpack object from a rvalue value, and reference bit length
        /// @param    v: referenced value
        /// @param    b: number of bits to pack into
        bitpack(T &&v, const size_t &b) noexcept : value{v}, bits{b} {}
    };

    /// @brief a container for fixed or dynamically sized arrays, with an upper bounds limit for safety
    /// @tparam   T: the array element type
    /// @tparam   ST: the type of the variable used by reference to adjust and report the array size
    template <typename T, typename ST = size_t>
    struct array
    {
        /// @brief a pointer to the array head
        T *value;

        /// @brief a reference to a value that contains the dynamic size of the array
        const ST &size;

        /// @brief maximum size of the array (used for bounds checking)
        const size_t max_size;

        /// @brief Construct a new array object from an existing array and a referenced size
        /// @tparam   max_elements: maximum number of elements in the array
        /// @param    v: reference to the underlying array
        /// @param    s: reference to a value representing the dynamic array size
        template <size_t max_elements>
        array(T (&v)[max_elements], const ST &s) noexcept : value{&v[0]}, size{s}, max_size{max_elements} {}

        /// @brief Construct a new array object from an existing array and a rvalue size
        /// @tparam   max_elements
        /// @param    v: reference to the underlying array
        /// @param    s: a rvalue representing the dynamic array size
        template <size_t max_elements>
        array(T (&v)[max_elements], const ST &&s) noexcept : value{&v[0]}, size{std::move(s)}, max_size{max_elements} {}

        /// @brief Construct a new array object from a pointer to the head of the array, a referenced dynamic size,
        /// and a constant max size
        /// @param    v: pointer to the head of the array
        /// @param    s: reference to a value representing the dynamic array size
        /// @param    max_elements: maximum size of the array
        array(T *v, ST &s, const size_t max_elements) noexcept : value{&v[0]}, size{s}, max_size{max_elements} {}

        /// @brief Construct a new array object from a pointer to the head of the array, a rvalue dynamic size,
        /// and a constant max size
        /// @param    v: pointer to the head of the array
        /// @param    s: rvalue representing the dynamic array size
        /// @param    max_elements: maximum size of the array
        array(T *v, const ST &&s, const size_t max_elements) noexcept : value{&v[0]}, size{std::move(s)}, max_size{max_elements} {}
    };

    /// @brief a container for dynamically sized arrays with their ending marked by a reserved delimiter value
    /// @tparam   T: the type of an array element
    template <typename T>
    struct delimited_array
    {
        /// @brief pointer to the head of the array
        T *value;

        /// @brief reference to a reserved delimiter value marking the end of the array
        const T &delimiter;

        /// @brief maximum size of the array (used for bounds checking)
        const size_t max_size;

        /// @brief Construct a new delimited array object using a referenced array and referenced delimiter
        /// @tparam   max_elements: maximum size of the array
        /// @param    v: reference to the underlying array
        /// @param    d: the delimiter value
        template <size_t max_elements>
        delimited_array(T (&v)[max_elements], T &d) noexcept : value{&v[0]}, delimiter{d}, max_size{max_elements} {}

        /// @brief Construct a new delimited array object using a referenced array and rvalue delimiter
        /// @tparam   max_elements: maximum size of the array
        /// @param    v: reference to the underlying array
        /// @param    d: the delimiter value
        template <size_t max_elements>
        delimited_array(T (&v)[max_elements], const T &&d) noexcept : value{&v[0]}, delimiter{std::move(d)}, max_size{max_elements} {}

        /// @brief Construct a new delimited array object using a pointer to an array, a referenced delimiter, and a max size
        /// @param    v: pointer to the underlying array
        /// @param    d: the delimiter value
        /// @param    max_elements: maximum size of the array
        delimited_array(T *v, T &d, size_t max_elements) noexcept : value{&v[0]}, delimiter{d}, max_size{max_elements} {}

        /// @brief Construct a new delimited array object using a pointer to an array, a rvalue delimiter, and a max size
        /// @param    v: pointer to the underlying array
        /// @param    d: the delimiter value
        /// @param    max_elements: maximum size of the array
        delimited_array(T *v, const T &&d, size_t max_elements) noexcept : value{&v[0]}, delimiter{std::move(d)}, max_size{max_elements} {}
    };

    struct packet_base;
    struct formatter;
    namespace detail
    {
        template <typename T>
        struct default_bitsize<bitpack<T>>
        {
            static constexpr size_t value = 0u;
        };
        template <typename T, typename T2>
        struct default_bitsize<array<T, T2>>
        {
            static constexpr size_t value = sizeof(T) * 8u;
        };
        template <typename T>
        struct default_bitsize<delimited_array<T>>
        {
            static constexpr size_t value = sizeof(T) * 8u;
        };
        template <>
        struct default_bitsize<align>
        {
            static constexpr size_t value = 0u;
        };
        template <>
        struct default_bitsize<pad>
        {
            static constexpr size_t value = 0u;
        };
        template <typename T>
        struct is_format_modifier
        {
            static constexpr bool value = std::is_base_of<packet_base, T>::value;
        };
        template <>
        struct is_format_modifier<formatter> : std::true_type
        {
        };
        template <>
        struct is_format_modifier<pad> : std::true_type
        {
        };
        template <>
        struct is_format_modifier<align> : std::true_type
        {
        };
        template <typename T>
        struct is_format_modifier<bitpack<T>> : std::true_type
        {
        };
        template <typename T1, typename T2>
        struct is_format_modifier<array<T1, T2>> : std::true_type
        {
        };
        template <typename T>
        struct is_format_modifier<delimited_array<T>> : std::true_type
        {
        };
    }
}

#endif // _SERDES_FORMAT_MODIFIERS_H_
