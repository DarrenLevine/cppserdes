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
    /// @tparam   ST: type of the align value
    template <typename ST>
    struct align
    {
        /// bit alignment reference value
        const ST &value;

        /// @brief Construct a new align object, binding to a reference of the number of bits to align by
        /// @param    bits
        constexpr align(const ST &bits) noexcept : value{bits} {}

        /// @brief Construct a new align object, binding to a rvalue of the number of bits to align by
        /// @param    bits
        constexpr align(ST &&bits) noexcept : value{bits} {}
    };

    /// @brief specifies a number of bits to be used to pad (add to) the current bit offset
    /// @tparam   ST: type of the pad value
    template <typename ST>
    struct pad
    {
        /// bit pad reference value
        const ST &value;

        /// @brief Construct a new pad object, binding to a reference of the number of bits to pad by
        /// @param    bits
        constexpr pad(const ST &bits) noexcept : value{bits} {}

        /// @brief Construct a new pad object, binding to a rvalue of the number of bits to pad by
        /// @param    bits
        constexpr pad(ST &&bits) noexcept : value{bits} {}
    };

    /// @brief bitpack a value into an exact number of specified bits. If applied to an array
    /// it will be applied to each element of the array, not the total bits in the array
    /// @tparam   T: the type of the value bitpacked
    /// @tparam   ST: the size type of the bit length value
    template <typename T, typename ST>
    struct bitpack
    {
        /// @brief a reference to the bitpacked value
        T &value;

        /// @brief a reference to the number of bits to use when storing/loading the referenced value
        const ST &bits;

        /// @brief Construct a new bitpack object from a rvalue value, and rvalue bit length
        /// @param    v: referenced value
        /// @param    b: number of bits to pack into
        constexpr bitpack(T &&v, const ST &&b) noexcept : value{v}, bits{std::move(b)} {}

        /// @brief Construct a new bitpack object from a reference value, and rvalue bit length
        /// @param    v: referenced value
        /// @param    b: number of bits to pack into
        constexpr bitpack(T &v, const ST &&b) noexcept : value{v}, bits{std::move(b)} {}

        /// @brief Construct a new bitpack object from a reference value, and reference bit length
        /// @param    v: referenced value
        /// @param    b: number of bits to pack into
        constexpr bitpack(T &v, const ST &b) noexcept : value{v}, bits{b} {}

        /// @brief Construct a new bitpack object from a rvalue value, and reference bit length
        /// @param    v: referenced value
        /// @param    b: number of bits to pack into
        constexpr bitpack(T &&v, const ST &b) noexcept : value{v}, bits{b} {}
    };

    /// @brief a container for fixed or dynamically sized arrays, with an upper bounds limit for safety
    /// @tparam   T: the array element type
    /// @tparam   ST: the type of the variable used by reference to adjust and report the array size
    template <typename T, typename ST>
    struct array
    {
        /// if the array is from a container with a "P* T.data()" method, elem_type will be P
        /// otherwise, elem_type will be T, allowing support for c-arrays (no data method) and
        /// std::array, std::list, std::vector, etc. (any c-array abstraction layer with a data() method)
        using elem_type = typename detail::has_data_and_size<T>::elem_type;

        /// @brief a pointer to the array head
        elem_type *value;

        /// @brief holds onto an rvalue size if provide (so that const ST &size can have a local reference)
        const ST rvalue_size;

        /// @brief a reference to a value that contains the dynamic size of the array
        /// for fixed sized arrays (where the size term is a rvalue temp), size will just refer to max_size since
        /// they're the same in that case if rvalue <= size, otherwise max will be set to 0 to force as size error
        const ST &size;

        /// @brief maximum size of the array (used for bounds checking)
        const size_t max_size;

        /// @brief Construct a new array object from an existing array and a referenced size
        /// @tparam   max_elements: maximum number of elements in the array
        /// @param    v: reference to the underlying array
        /// @param    s: reference to a value representing the dynamic array size
        template <size_t max_elements>
        constexpr array(T (&v)[max_elements], const ST &s) noexcept : value{&v[0]}, rvalue_size{}, size{s}, max_size{max_elements} {}

        /// @brief Construct a new array object from an existing array and a rvalue size
        /// @tparam   max_elements
        /// @param    v: reference to the underlying array
        /// @param    s: a rvalue representing the dynamic array size
        template <size_t max_elements>
        constexpr array(T (&v)[max_elements], const ST &&s) noexcept : value{&v[0]}, rvalue_size{s}, size{rvalue_size}, max_size(max_elements) {}

        /// @brief Construct a new array object from a pointer to the head of the array, a referenced dynamic size,
        /// and a constant max size
        /// @param    v: pointer to the head of the array
        /// @param    s: reference to a value representing the dynamic array size
        /// @param    max_elements: maximum size of the array
        constexpr array(T *v, ST &s, const ST max_elements) noexcept : value{&v[0]}, rvalue_size{}, size{s}, max_size{max_elements} {}

        /// @brief Construct a new array object from a pointer to the head of the array, a rvalue dynamic size,
        /// and a constant max size
        /// @param    v: pointer to the head of the array
        /// @param    s: rvalue representing the dynamic array size
        /// @param    max_elements: maximum size of the array
        constexpr array(T *v, const ST &&s, const ST max_elements) noexcept : value{&v[0]}, rvalue_size{s}, size{rvalue_size}, max_size{max_elements} {}

        /// @brief Construct a new "dynamically" sized array object from a data container with 'T* data()' and 'size_t size()'
        /// methods (such as std::array, or std::list)
        ///
        /// @param    v: reference to the data container whose underlying data should be used as the array
        /// @param    s: reference to a value representing the "dynamic" array size
        array(T &v, const ST &s) noexcept : value{v.data()}, rvalue_size{}, size{s}, max_size(v.size()) {}

        /// @brief Construct a new "fixed" sized array object from a data container with 'T* data()' and 'size_t size()' 
        /// methods (such as std::array, or std::list)
        ///
        /// @param    v: reference to the data container whose underlying data should be used as the array
        /// @param    s: rvalue representing the "fixed" array size
        array(T &v, const ST &&s) noexcept : value{v.data()}, rvalue_size{s}, size{rvalue_size}, max_size(v.size()) {}

        /// @brief Construct a new "fixed" sized array object from a data container with 'T* data()' and 'size_t size()'
        /// methods (such as std::array, or std::list)
        ///
        /// @param    v: reference to the data container whose underlying data should be used as the array
        array(T &v) noexcept : value{v.data()}, rvalue_size{max_size}, size{rvalue_size}, max_size(v.size()) {}
    };

    /// @brief a container for dynamically sized arrays with their ending marked by a reserved delimiter value
    /// @tparam   T: the type of an array element
    template <typename T>
    struct delimited_array
    {
        /// if the array is from a container with a "P* T.data()" method, elem_type will be P
        /// otherwise, elem_type will be T, allowing support for c-arrays (no data method) and
        /// std::array, std::list, std::vector, etc. (any c-array abstraction layer with a data() method)
        using elem_type = typename detail::has_data_and_size<T>::elem_type;

        /// @brief pointer to the head of the array
        elem_type *value;

        /// @brief reference to a reserved delimiter value marking the end of the array
        const elem_type &delimiter;

        /// @brief maximum size of the array (used for bounds checking)
        const size_t max_size;

        /// @brief Construct a new delimited array object using a referenced array and referenced delimiter
        /// @tparam   max_elements: maximum size of the array
        /// @param    v: reference to the underlying array
        /// @param    d: the delimiter value
        template <size_t max_elements>
        constexpr delimited_array(T (&v)[max_elements], const T &d) noexcept : value{&v[0]}, delimiter{d}, max_size{max_elements} {}

        /// @brief Construct a new delimited array object using a referenced array and rvalue delimiter
        /// @tparam   max_elements: maximum size of the array
        /// @param    v: reference to the underlying array
        /// @param    d: the delimiter value
        template <size_t max_elements>
        constexpr delimited_array(T (&v)[max_elements], const T &&d) noexcept : value{&v[0]}, delimiter{std::move(d)}, max_size{max_elements} {}

        /// @brief Construct a new delimited array object using a pointer to an array, a referenced delimiter, and a max size
        /// @param    v: pointer to the underlying array
        /// @param    d: the delimiter value
        /// @param    max_elements: maximum size of the array
        constexpr delimited_array(T *v, const T &d, size_t max_elements) noexcept : value{&v[0]}, delimiter{d}, max_size{max_elements} {}

        /// @brief Construct a new delimited array object using a pointer to an array, a rvalue delimiter, and a max size
        /// @param    v: pointer to the underlying array
        /// @param    d: the delimiter value
        /// @param    max_elements: maximum size of the array
        constexpr delimited_array(T *v, const T &&d, size_t max_elements) noexcept : value{&v[0]}, delimiter{std::move(d)}, max_size{max_elements} {}

        /// @brief Construct a new delimited array object from a data container with 'T* data()' and 'size_t size()'
        /// methods (such as std::array, or std::list)
        ///
        /// @param    v: reference to the data container whose underlying data should be used as the array
        /// @param    d: a reference to the delimiter value
        delimited_array(T &v, const elem_type &d) noexcept : value{v.data()}, delimiter{d}, max_size(v.size()) {}

        /// @brief Construct a new delimited array object from a data container with 'T* data()' and 'size_t size()'
        /// methods (such as std::array, or std::list)
        ///
        /// @param    v: reference to the data container whose underlying data should be used as the array
        /// @param    d: a rvalue reference to the delimiter value
        delimited_array(T &v, const elem_type &&d) noexcept : value{v.data()}, delimiter{std::move(d)}, max_size(v.size()) {}
    };

    struct packet_base;
    struct formatter;
    template <typename FieldType, typename FuncType>
    struct validator;
    namespace detail
    {
        template <typename T, typename ST>
        struct default_bitsize<bitpack<T, ST>>
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
        template <typename ST>
        struct default_bitsize<align<ST>>
        {
            static constexpr size_t value = 0u;
        };
        template <typename ST>
        struct default_bitsize<pad<ST>>
        {
            static constexpr size_t value = 0u;
        };
        template <typename T>
        struct is_formatter : std::false_type
        {
        };
        template <>
        struct is_formatter<formatter> : std::true_type
        {
        };
        template <typename T>
        struct is_validator : std::false_type
        {
        };
        template <typename FieldType, typename FuncType>
        struct is_validator<validator<FieldType, FuncType>> : std::true_type
        {
        };
        template <typename T>
        struct is_format_modifier
        {
            static constexpr bool value = has_format_method<T>::value;
        };
        template <>
        struct is_format_modifier<formatter> : std::true_type
        {
        };
        template <typename ST>
        struct is_format_modifier<pad<ST>> : std::true_type
        {
        };
        template <typename ST>
        struct is_format_modifier<align<ST>> : std::true_type
        {
        };
        template <typename T, typename ST>
        struct is_format_modifier<bitpack<T, ST>> : std::true_type
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
