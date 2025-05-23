/// @file serdes.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief includes all serdes namespace functionality (bitcpy, packet, packet_base, etc.) except
/// stdio related printing functions (import bitprint.h separately for those)
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _SERDES_H_
#define _SERDES_H_

#include <cstring>
#include "bitcpy.h"
#include "bitliterals.h"
#include "serdes_errors.h"
#include "serdes_formatter.h"
#include "serdes_validator.h"
#include "serdes_format_modifiers.h"
#include "serdes_fwd_declarations.h"
#include "serdes_byte_iterator.h"

// define configCPP_SERDES_LIB_EXCLUDE_CPP_CRC to skip including "cppcrc.h"
// NOTE: cppcrc.h relies on C++14 or greater constexpr support
#if !defined(configCPP_SERDES_LIB_EXCLUDE_CPP_CRC) && BITCPY_CONSTEXPR_SUPPORTED
#include "cppcrc.h"
#endif

/// @brief CppSerdes library namespace
namespace serdes
{
    /// @brief a serialization/deserialization helper class, with load, store, and stream operators
    struct packet
    {
        serdes::sized_pointer<void> buffer;   ///< underlying buffer to serialize/deserialize to/from
        size_t bit_offset;                    ///< current bit offset in the serdes process
        mode_e mode;                          ///< the serdes mode (LOADING, STORING, or UNSPECIFIED)
        status_e status = status_e::NO_ERROR; ///< the current error status of the serdes process
        const size_t bit_capacity;            ///< buffer.bit_capacity() value

        /// @brief resets the bit offset to 0 and the status to NO_ERROR
        inline void reset() noexcept
        {
            status = status_e::NO_ERROR;
            bit_offset = 0;
        }

        /// @brief Construct a new packet object from an c style array pointer
        /// @tparam   T_pointer: the array's base pointer type
        /// @param    array_init: an array pointer (without size)
        /// @param    max_elements: the maximum number of elements to use in the array
        /// @param    b_offset: bit offset to start serdes process at
        /// @param    m: starting mode (LOADING/STORING/UNSPECIFIED)
        template <typename T_pointer, typename std::enable_if<std::is_pointer<T_pointer>::value, int *>::type = nullptr>
        packet(T_pointer array_init, size_t max_elements = ~size_t(0), size_t b_offset = 0, mode_e m = mode_e::UNSPECIFIED)
            : buffer{array_init, max_elements},
              bit_offset{b_offset},
              mode{m},
              bit_capacity{buffer.bit_capacity()} {}

        /// @brief Construct a new packet object from an c style array pointer
        /// @tparam   T_array: the array's base type
        /// @param    array_init: an array pointer (without size)
        /// @param    max_elements: the maximum number of elements to use in the array (can be <= N)
        /// @param    b_offset: bit offset to start serdes process at
        /// @param    m: starting mode (LOADING/STORING/UNSPECIFIED)
        template <typename T_array, size_t N>
        packet(T_array (&array_init)[N], size_t max_elements = ~size_t(0), size_t b_offset = 0, mode_e m = mode_e::UNSPECIFIED)
            : buffer{array_init, max_elements < N ? max_elements : N},
              bit_offset{b_offset},
              mode{m},
              bit_capacity{buffer.bit_capacity()} {}

        /// @brief Construct a new packet object from an sized_pointer array for size safety
        /// @tparam   T_sized_pointer: the array's sized_pointer type
        /// @param    array_init: an array with size information
        /// @param    b_offset: bit offset to start serdes process at
        /// @param    m: starting mode (LOADING/STORING/UNSPECIFIED)
        template <typename T_sized_pointer, typename std::enable_if<serdes::detail::is_sized_pointer<T_sized_pointer>::value, int *>::type = nullptr>
        packet(T_sized_pointer array_init, size_t b_offset = 0, mode_e m = mode_e::UNSPECIFIED)
            : buffer(array_init),
              bit_offset{b_offset},
              mode{m},
              bit_capacity{buffer.bit_capacity()} {}

        /// @brief Construct a new packet object from a reference to any container with a data() and size() method
        /// For example, a std::array, or a std::vector
        /// @tparam   T_array: the container type
        /// @param    array_init: the container to reference
        /// @param    max_elements: the maximum number of elements to use in the array (can be <= size())
        /// @param    b_offset: bit offset to start serdes process at
        /// @param    m: starting mode (LOADING/STORING/UNSPECIFIED)
        template <typename T_array, detail::requires_data_and_size<T_array>* = nullptr>
        packet(T_array &array_init, size_t max_elements = ~size_t(0), size_t b_offset = 0, mode_e m = mode_e::UNSPECIFIED)
            : buffer{array_init.data(), max_elements < array_init.size() ? max_elements : array_init.size()},
              bit_offset{b_offset},
              mode{m},
              bit_capacity{buffer.bit_capacity()} {}

        /// @brief moves the bit offset head the specified bits
        /// @param    bits: number of bits to pad
        void pad(const size_t bits)
        {
            if (status == status_e::NO_ERROR)
                pad_assuming_no_prior_errors(bits);
        }

        /// @brief aligns (increases) the bit offset to a multiple of the specified bits
        /// @param    bits: number of bits to align to
        void align(size_t bits)
        {
            if (status == status_e::NO_ERROR)
                align_assuming_no_prior_errors(bits);
        }

        //
        // LOAD SECTION (load from serial = deserialize)
        //

        /// @brief [[deserialize]] loads from serial buffer into the passed array of values
        /// @tparam   T: type of the target value to load into
        /// @tparam   N: maximum number of values to load into
        /// @param    value: target value to load into
        /// @param    bits: number of bits per value element (not of the entire array)
        template <typename T, size_t N>
        void load(T (&value)[N], size_t bits = detail::default_bitsize<T>::value)
        {
            array<T, size_t> temp_arr(value, N);
            load(temp_arr, bits);
        }

        /// @brief [[deserialize]] loads from serial buffer into the passed reference to a
        /// data container (any object with a size() and data() method).
        /// @tparam   T: type of the target value to load into
        /// @param    value: target value to load into
        /// @param    bits: number of bits per element of the container (not of the entire container)
        template <typename T, typename std::enable_if<
            detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::value &&
            !detail::has_custom_type_override<typename detail::remove_cvref_cpp11<T>::type>::value
            , int *>::type = nullptr>
        void load(T &&value, size_t bits = detail::default_bitsize<typename detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::elem_type>::value)
        {
            if (status != status_e::NO_ERROR)
                return;
            size_t size = value.size();
            if (size == 0u) // no space to load
            {
                if (bits > 0u)
                    status = status_e::ARRAY_SIZE_OVER_MAX;
                return;
            }
            using elem_type = typename detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::elem_type;
            array<elem_type, size_t> temp_arr(&value.data()[0], size, size);
            load(temp_arr, bits);
        }

        /// @brief [[deserialize]] loads from serial buffer into the passed value reference
        /// @tparam   T: type of the target value to load into
        /// @param    value: target value to load into
        /// @param    bits: number of bits per value
        template <typename T, typename std::enable_if<
            !detail::is_format_modifier<typename detail::remove_cvref_cpp11<T>::type>::value &&
            detail::supported_by_bitcpy<typename detail::remove_cvref_cpp11<T>::type>::value,
                                       int *>::type = nullptr>
        void load(T &value, size_t bits = detail::default_bitsize<T>::value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            const size_t bits_touched = bitcpy(value, buffer, bit_offset, bits);
            bit_offset += bits_touched;
            if (bits_touched < bits)
                status = status_e::EXCEEDED_SERIAL_SIZE;
        }

        /// @brief [[deserialize]] loads from serial buffer into a "custom_type" overriden type,
        /// specifically when it supports bitpacking support (accepts bits as an argument and defines
        /// a default bit size value for the item).
        /// @tparam   T : A type with a custom_type<..> definition for deserialization.
        /// @param    value : the custom_type value
        /// @param    bits : the number of bits to pack the item into
        template <typename T, typename std::enable_if<
            detail::has_custom_type_with_bit_support<typename detail::remove_cvref_cpp11<T>::type>::value 
            && (
                // and has an override
                detail::has_custom_type_override<typename detail::remove_cvref_cpp11<T>::type>::value ||
                // or, not already supported by another overload
                (
                    !detail::is_format_modifier<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::supported_by_bitcpy<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !std::is_array<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::has_load_and_store<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::value
                )
            ), int *>::type = nullptr>
        void load(T &&value, size_t bits = custom_type<typename detail::remove_cvref_cpp11<T>::type>::default_bits)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            custom_type<typename detail::remove_cvref_cpp11<T>::type>::format(*this, std::forward<T>(value), bits);
        }

        /// @brief [[deserialize]] loads from serial buffer into a "custom_type" overriden type,
        /// specifically when it does not support bitpacking (does not use bits as an argument).
        /// @tparam   T : A type with a custom_type<..> definition for deserialization.
        /// @param    value : the custom_type value
        /// @param    bits : the number of bits to pack the item into
        template <typename T, typename std::enable_if<
            detail::has_custom_type_without_bit_support<typename detail::remove_cvref_cpp11<T>::type>::value 
            && (
                // and has an override
                detail::has_custom_type_override<typename detail::remove_cvref_cpp11<T>::type>::value ||
                // or, not already supported by another overload
                (
                    !detail::is_format_modifier<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::supported_by_bitcpy<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !std::is_array<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::has_load_and_store<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::value
                )
            ), int *>::type = nullptr>
        void load(T &&value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            custom_type<typename detail::remove_cvref_cpp11<T>::type>::format(*this, std::forward<T>(value));
        }

        /// @brief [[deserialize]] loads from serial buffer into type that has a "void format(serdes::packet&)"
        /// method.
        /// @tparam   T : A type with a "void format(serdes::packet&)" method (does not need to be from packet_base)
        /// @param    value : the object to write serial data to
        template <typename T, typename std::enable_if<detail::has_format_method<T>::value, int *>::type = nullptr>
        void load(T &&value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            std::forward<T>(value).format(*this);
        }

        /// @brief [[deserialize]] loads from serial buffer into a type that has both a "T load()" and "void store(T)"
        /// method.
        /// @tparam   T : Any type with "T load()" and "void store(T)" methods
        /// @param    value : the object to write serial data to
        template <typename T, typename std::enable_if<detail::has_load_and_store<typename detail::remove_cvref_cpp11<T>::type>::value, int *>::type = nullptr>
        void load(T &&value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            typename detail::get_inner_wrapped_type<typename detail::remove_cvref_cpp11<T>::type>::type temp_value{};
            load(temp_value);
            if (status != status_e::NO_ERROR)
                value.store(temp_value);
        }

        /// @brief [[deserialize]] loads from serial buffer into the passed value rvalue
        /// NOTE: using this method will cause a status_e::NO_LOAD_TO_RVALUE error
        template <typename T, typename std::enable_if<
            std::is_rvalue_reference<T &&>::value
            // don't set this NO_LOAD_TO_RVALUE warning for any complex types whose rvalue
            // could wrap a valid lvalue variable such as "packet >> serdes::array(data);"
            && detail::supported_by_bitcpy<T>::value,
            int *>::type = nullptr>
        void load(T &&value, size_t bits = detail::default_bitsize<T>::value)
        {
            (void)value;
            (void)bits;
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            status = status_e::NO_LOAD_TO_RVALUE;
        }

        /// @brief [[deserialize]] loads from serial buffer into a formatter reference
        /// @param    value: target formatter reference to load into
        template<typename T, typename std::enable_if<
            detail::is_formatter<typename detail::remove_cvref_cpp11<T>::type>::value,
            int *>::type* = nullptr>
        void load(T &&value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            if (value.formatter_lambda == nullptr)
            {
                status = status_e::FORMATTER_NOT_SET;
                return;
            }
            value.formatter_lambda(*this);
        }

        /// @brief [[deserialize]] loads from serial buffer into a delimited_array reference
        /// @tparam   T: underlying type of the delimited_array
        /// @param    value: referenced delimited_array
        /// @param    bits: number of bits per element
        template <typename T, typename std::enable_if<!detail::is_format_modifier<typename delimited_array<T>::elem_type>::value, int *>::type = nullptr>
        void load(delimited_array<T> &value, size_t bits = detail::default_bitsize<typename delimited_array<T>::elem_type>::value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            using elem_type = typename delimited_array<T>::elem_type;
            constexpr size_t bits_per_element = sizeof(elem_type) * 8;

            // shortcut for memory aligned situations
            if (bits == bits_per_element && buffer.element_size == sizeof(elem_type) && bit_offset % bits_per_element == 0)
            {
                if (bit_capacity < bits_per_element)
                {
                    status = status_e::EXCEEDED_SERIAL_SIZE;
                    return;
                }
                const size_t max_bit_offset_minus_one_element = bit_capacity - bits_per_element;
                const elem_type *buffer_head = &reinterpret_cast<const elem_type *>(buffer.value)[bit_offset / bits_per_element];
                for (size_t i = 0; i < value.max_size; i++)
                {
                    if (bit_offset > max_bit_offset_minus_one_element)
                    {
                        status = status_e::EXCEEDED_SERIAL_SIZE;
                        return;
                    }
                    const elem_type &source_value = *buffer_head;
                    value.value[i] = source_value;
                    bit_offset += bits;
                    if (source_value == value.delimiter)
                        return;
                    ++buffer_head;
                }
            }
            else
            {
                for (size_t i = 0; i < value.max_size; i++)
                {
                    const size_t bits_touched = bitcpy(value.value[i], buffer, bit_offset, bits);
                    bit_offset += bits_touched;
                    if (bits_touched < bits)
                    {
                        status = status_e::EXCEEDED_SERIAL_SIZE;
                        return;
                    }
                    if (value.value[i] == value.delimiter)
                        return;
                }
            }
            status = status_e::DELIMITER_NOT_FOUND;
        }

        /// @brief [[deserialize]] loads from serial buffer into a delimited_array<InnerType> reference
        /// where the InnerType has a "void format(packet&)" method.
        /// @tparam   T: underlying type of the delimited_array
        /// @param    value: referenced delimited_array
        template <typename T, typename std::enable_if<detail::has_format_method<typename delimited_array<T>::elem_type>::value, int *>::type = nullptr>
        void load(delimited_array<T> &value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            for (size_t i = 0; i < value.max_size; i++)
            {
                value.value[i].format(*this);
                if (value.value[i] == value.delimiter || status != status_e::NO_ERROR)
                    return;
            }
            status = status_e::DELIMITER_NOT_FOUND;
        }

        /// @brief [[deserialize]] loads from serial buffer into a delimited_array<T> reference
        /// @tparam   T: underlying type of the delimited_array
        /// @param    value: referenced delimited_array
        template <typename T>
        void load(delimited_array<T> &&value)
        {
            load(value);
        }

        /// @brief [[deserialize]] loads from serial buffer into a array<T1, T2> reference
        /// @tparam   T: underlying type of the array
        /// @tparam   T2: underlying size type of the array
        /// @param    value: referenced array
        /// @param    bits: number of bits per element
        template <typename T, typename T2, typename std::enable_if<
            !detail::is_format_modifier<typename serdes::array<T, T2>::elem_type>::value &&
            detail::supported_by_bitcpy<typename serdes::array<T, T2>::elem_type>::value
            , int *>::type = nullptr>
        void load(serdes::array<T, T2> &value, size_t bits = detail::default_bitsize<typename serdes::array<T, T2>::elem_type>::value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();

            // here the "size" reference is finally copied, in case it changed after the reference bind occurred
            size_t array_size = static_cast<size_t>(value.size);
            if (array_size > value.max_size)
            {
                array_size = value.max_size;
                status = status_e::ARRAY_SIZE_OVER_MAX;
            }
            const size_t total_bits = array_size * sizeof(typename serdes::array<T, T2>::elem_type) * 8;
            // shortcut for memory aligned situations
            if (sizeof(typename serdes::array<T, T2>::elem_type) == 1 && bits == 8 && buffer.element_size == 1 && (bit_offset & 7u) == 0u && bit_capacity - bit_offset >= total_bits)
            {
                std::memcpy(&value.value[0], &reinterpret_cast<uint8_t *>(buffer.value)[bit_offset >> 3], array_size);
                bit_offset += total_bits;
                return;
            }
            for (size_t i = 0; i < array_size; i++)
            {
                const size_t bits_touched = bitcpy(value.value[i], buffer, bit_offset, bits);
                bit_offset += bits_touched;
                if (bits_touched < bits)
                {
                    status = status_e::EXCEEDED_SERIAL_SIZE;
                    return;
                }
            }
        }

        /// @brief [[deserialize]] loads from serial buffer into a array<T1, T2> reference
        /// @tparam   T: underlying type of the array
        /// @tparam   T2: underlying size type of the array
        /// @param    value: referenced array
        /// @param    bits: number of bits per element
        template <typename T, typename T2, typename std::enable_if<
            !detail::is_format_modifier<typename serdes::array<T, T2>::elem_type>::value &&
            !detail::supported_by_bitcpy<typename serdes::array<T, T2>::elem_type>::value
            , int *>::type = nullptr>
        void load(serdes::array<T, T2> &value, size_t bits = detail::default_bitsize<typename serdes::array<T, T2>::elem_type>::value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();

            // here the "size" reference is finally copied, in case it changed after the reference bind occurred
            size_t array_size = static_cast<size_t>(value.size);
            if (array_size > value.max_size)
            {
                array_size = value.max_size;
                status = status_e::ARRAY_SIZE_OVER_MAX;
            }
            for (size_t i = 0; i < array_size && status != status_e::NO_ERROR; i++)
                load(value.value[i], bits);
        }

        /// @brief [[deserialize]] loads from serial buffer into a array<packet_base, T2> reference
        /// @tparam   T: underlying type of the array (must be derived from packet_base or have a "void format(packet&)" method)
        /// @tparam   T2: underlying size type of the array
        /// @param    value: referenced array
        template <typename T, typename T2, typename std::enable_if<detail::has_format_method<typename serdes::array<T, T2>::elem_type>::value, int *>::type = nullptr>
        void load(serdes::array<T, T2> &value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();

            // here the "size" reference is finally copied, in case it changed after the reference bind occurred
            size_t array_size = static_cast<size_t>(value.size);
            if (array_size > value.max_size)
            {
                status = status_e::ARRAY_SIZE_OVER_MAX;
                return;
            }
            for (size_t i = 0; i < array_size; i++)
            {
                value.value[i].format(*this);
                if (status != status_e::NO_ERROR)
                    return;
            }
        }

        /// @brief [[deserialize]] loads from serial buffer into a array<T, T2> forwarding reference
        /// @tparam   T: underlying type of the array
        /// @tparam   T2: underlying size type of the array
        /// @param    value: referenced array
        template <typename T, typename T2>
        void load(serdes::array<T, T2> &&value)
        {
            load(value);
        }

        /// @brief [[deserialize, pad]] applies a padding step to the load process
        /// @tparam   ST: size type of the pad
        /// @param    padding: pad value
        template <typename ST>
        void load(const serdes::pad<ST> padding)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            pad_assuming_no_prior_errors(padding.value);
        }

        /// @brief [[deserialize, align]] applies an alignment step to the load process
        /// @tparam   ST: size type of the align
        /// @param    alignment: align value
        template <typename ST>
        void load(const serdes::align<ST> alignment)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            align_assuming_no_prior_errors(alignment.value);
        }

        /// @brief [[deserialize]] loads from serial buffer into a bitpack<T, ST> rvalue
        /// @tparam   T,: underlying type of the bitpack object
        /// @tparam   ST: underlying size type of the bitpack object
        /// @param    value: referenced bitpack rvalue
        template <typename T, typename ST>
        void load(bitpack<T, ST> &&value)
        {
            load(value.value, value.bits);
        }

        /// @brief [[deserialize]] loads from serial buffer into a bitpack<T, ST> reference
        /// @tparam   T: underlying type of the bitpack object
        /// @tparam   ST: underlying size type of the bitpack object
        /// @param    value: referenced bitpack reference
        template <typename T, typename ST>
        void load(bitpack<T, ST> &value)
        {
            load(value.value, value.bits);
        }

        /// @brief [[deserialize]] loads from serial buffer into a packet_base reference
        /// @param    value: referenced packet_base reference
        void load(packet_base &value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_load();
            value.format(*this);
        }

        /// @brief [[deserialize]] loads from serial buffer into a packet_base rvalue
        /// @param    value: referenced packet_base rvalue
        void load(packet_base &&value)
        {
            load(value);
        }

        //
        // STORE SECTION (store into serial = serialize)
        //

        /// @brief [[serialize]] stores a value reference into a serial buffer
        /// @tparam   T: value type
        /// @param    value: value to store
        /// @param    bits: bits per value
        template <typename T, typename std::enable_if<
            !detail::is_format_modifier<typename detail::remove_cvref_cpp11<T>::type>::value &&
            detail::supported_by_bitcpy<typename detail::remove_cvref_cpp11<T>::type>::value,
                                  int *>::type = nullptr>
        void store(const T &value, size_t bits = detail::default_bitsize<T>::value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            const size_t bits_touched = bitcpy(buffer, value, bit_offset, bits);
            bit_offset += bits_touched;
            if (bits_touched < bits)
                status = status_e::EXCEEDED_SERIAL_SIZE;
        }

        /// @brief [[serialize]] stores a "custom_type" overriden type into a serial buffer,
        /// specifically when it supports bitpacking support (accepts bits as an argument and defines
        /// a default bit size value for the item).
        /// @tparam   T : A type with a custom_type<..> definition for deserialization.
        /// @param    value : the custom_type value
        /// @param    bits : the number of bits to unpack the item from
        template <typename T, typename std::enable_if<
            detail::has_custom_type_with_bit_support<typename detail::remove_cvref_cpp11<T>::type>::value 
            &&
            (
                // and has an override
                detail::has_custom_type_override<typename detail::remove_cvref_cpp11<T>::type>::value ||
                // or, not already supported by another overload
                (
                    !detail::is_format_modifier<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::supported_by_bitcpy<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !std::is_array<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::has_load_and_store<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::value
                )
            ), int *>::type = nullptr>
        void store(T &&value, size_t bits = custom_type<typename detail::remove_cvref_cpp11<T>::type>::default_bits)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            custom_type<typename detail::remove_cvref_cpp11<T>::type>::format(*this, std::forward<T>(value), bits);
        }

        /// @brief [[serialize]] stores a "custom_type" overriden type into a serial buffer,
        /// specifically when it does not support bitpacking (does not use bits as an argument).
        /// @tparam   T : A type with a custom_type<..> definition for deserialization.
        /// @param    value : the custom_type value
        template <typename T, typename std::enable_if<
            detail::has_custom_type_without_bit_support<typename detail::remove_cvref_cpp11<T>::type>::value 
            &&
            (
                // and has an override
                detail::has_custom_type_override<typename detail::remove_cvref_cpp11<T>::type>::value ||
                // or, not already supported by another overload
                (
                    !detail::is_format_modifier<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::supported_by_bitcpy<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !std::is_array<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::has_load_and_store<typename detail::remove_cvref_cpp11<T>::type>::value &&
                    !detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::value
                )
            ), int *>::type = nullptr>
        void store(T &&value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            custom_type<typename detail::remove_cvref_cpp11<T>::type>::format(*this, std::forward<T>(value));
        }

        /// @brief [[serialize]] stores a type that has a "void format(serdes::packet&)" method into a serial buffer.
        /// @tparam   T : A type with a "void format(serdes::packet&)" method (does not need to be from packet_base)
        /// @param    value : the object to turn into serial data
        template <typename T, typename std::enable_if<detail::has_format_method<T>::value, int *>::type = nullptr>
        void store(T &&value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            std::forward<T>(value).format(*this);
        }

        /// @brief [[serialize]] stores a type that has both "T load()" and "void store(T)" methods into a serial buffer.
        /// @tparam   T : Any type with "T load()" and "void store(T)" methods
        /// @param    value : the object to write serial data to
        template <typename T, typename std::enable_if<detail::has_load_and_store<
            typename detail::remove_cvref_cpp11<T>::type>::value, int *>::type = nullptr>
        void store(T &&value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            store(value.load());
        }

        /// @brief [[serialize]] stores a raw c-array reference into a serial buffer
        /// @tparam   T: array base type
        /// @tparam   N: array size
        /// @param    value: value to store
        /// @param    bits: bits per value element
        template <typename T, size_t N>
        void store(const T (&value)[N], size_t bits = detail::default_bitsize<T>::value)
        {
            store(array<const T, size_t>(value, N), bits);
        }

        /// @brief [[serialize]] stores any array-like container with "T* data()" and "size_t size()" methods
        /// into a serial buffer by treating it like a serdes::array. For example, a std::array, or std::vector.
        /// @tparam   T : Any type with both "T* data()" and "size_t size()" methods.
        /// @param    value: value to store
        /// @param    bits: bits per element of the array
        template <typename T, typename std::enable_if<
            detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::value &&
            !detail::has_custom_type_override<typename detail::remove_cvref_cpp11<T>::type>::value
            , int *>::type = nullptr>
        void store(T &&value, size_t bits = detail::default_bitsize<typename detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::elem_type>::value)
        {
            size_t size = value.size();
            if (size == 0u)
                return; // nothing to store
            using elem_type = typename detail::has_data_and_size<typename detail::remove_cvref_cpp11<T>::type>::elem_type;
            array<elem_type, size_t> temp_arr(&value.data()[0], size, size);
            store(temp_arr, bits);
        }

        /// @brief [[serialize]] stores a raw array rvalue into a serial buffer
        /// @tparam   T: array base type
        /// @tparam   N: array size
        /// @param    value: value to store
        /// @param    bits: bits per value element
        template <typename T, size_t N>
        void store(const T(&&value)[N], size_t bits = detail::default_bitsize<T>::value)
        {
            store(array<const T, size_t>(value, N), bits);
        }

        /// @brief [[serialize]] stores a delimited_array reference into a serial buffer
        /// @tparam   T: delimited_array base type
        /// @param    value: value to store
        /// @param    bits: bits per element of the array
        template <typename T, typename std::enable_if<!detail::is_format_modifier<typename delimited_array<T>::elem_type>::value, int *>::type = nullptr>
        void store(const delimited_array<T> &value, size_t bits = detail::default_bitsize<typename delimited_array<T>::elem_type>::value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            using elem_type = typename detail::remove_cvref_cpp11<typename delimited_array<T>::elem_type>::type;
            constexpr size_t bits_per_element = sizeof(elem_type) * 8;
            // shortcut for memory aligned situations
            if (bits == bits_per_element && buffer.element_size == sizeof(elem_type) && bit_offset % bits_per_element == 0)
            {
                if (bit_capacity < bits_per_element)
                {
                    status = status_e::EXCEEDED_SERIAL_SIZE;
                    return;
                }
                const size_t max_bit_offset_minus_one_element = bit_capacity - bits_per_element;
                elem_type *buffer_head = &reinterpret_cast<elem_type *>(buffer.value)[bit_offset / bits_per_element];
                for (size_t i = 0; i < value.max_size; i++)
                {
                    if (bit_offset > max_bit_offset_minus_one_element)
                    {
                        status = status_e::EXCEEDED_SERIAL_SIZE;
                        return;
                    }
                    const elem_type &source_value = value.value[i];
                    *buffer_head = source_value;
                    bit_offset += bits;
                    if (source_value == value.delimiter)
                        return;
                    ++buffer_head;
                }
            }
            else
            {
                for (size_t i = 0; i < value.max_size; i++)
                {
                    const size_t bits_touched = bitcpy(buffer, value.value[i], bit_offset, bits);
                    bit_offset += bits_touched;
                    if (bits_touched < bits)
                    {
                        status = status_e::EXCEEDED_SERIAL_SIZE;
                        return;
                    }
                    if (value.value[i] == value.delimiter)
                        return;
                }
            }
            status = status_e::DELIMITER_NOT_FOUND;
        }

        /// @brief [[serialize]] stores a delimited_array<packet_base> reference into a serial buffer
        /// @tparam   T: delimited_array base type (must be derived from packet_base or have a "void format(packet&)" method)
        /// @param    value: value to store
        template <typename T, typename std::enable_if<detail::has_format_method<typename delimited_array<T>::elem_type>::value, int *>::type = nullptr>
        void store(const delimited_array<T> &value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            for (size_t i = 0; i < value.max_size; i++)
            {
                value.value[i].format(*this);
                if (value.value[i] == value.delimiter || status != status_e::NO_ERROR)
                    return;
            }
            status = status_e::DELIMITER_NOT_FOUND;
        }

        /// @brief [[serialize]] stores a delimited_array<packet_base> rvalue into a serial buffer
        /// @tparam   T: delimited_array base type (must be derived from packet_base or have a "void format(packet&)" method)
        /// @param    value: value to store
        template <typename T>
        void store(const delimited_array<T> &&value)
        {
            store(value);
        }

        /// @brief [[serialize]] stores an array<T1, T2> reference into a serial buffer
        /// @tparam   T: array base type
        /// @tparam   T2: array size type
        /// @param    value: value to store
        /// @param    bits: bits per value element
        template <typename T, typename T2, typename std::enable_if<
            !detail::is_format_modifier<typename serdes::array<T, T2>::elem_type>::value &&
            detail::supported_by_bitcpy<typename serdes::array<T, T2>::elem_type>::value
            , int *>::type = nullptr>
        void store(const serdes::array<T, T2> &value, size_t bits = detail::default_bitsize<typename serdes::array<T, T2>::elem_type>::value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();

            // here the "size" reference is finally copied, in case it changed after the reference bind occurred
            size_t array_size = static_cast<size_t>(value.size);
            if (array_size > value.max_size)
            {
                array_size = value.max_size;
                status = status_e::ARRAY_SIZE_OVER_MAX;
            }
            const size_t total_bits = array_size * sizeof(typename serdes::array<T, T2>::elem_type) * 8;
            // shortcut for memory aligned situations
            if (sizeof(typename serdes::array<T, T2>::elem_type) == 1 && bits == 8 && buffer.element_size == 1 && (bit_offset & 7u) == 0u && bit_capacity - bit_offset >= total_bits)
            {
                std::memcpy(&reinterpret_cast<uint8_t *>(buffer.value)[bit_offset >> 3], &value.value[0], array_size);
                bit_offset += total_bits;
                return;
            }
            for (size_t i = 0; i < array_size; i++)
            {
                const size_t bits_touched = bitcpy(buffer, value.value[i], bit_offset, bits);
                bit_offset += bits_touched;
                if (bits_touched < bits)
                {
                    status = status_e::EXCEEDED_SERIAL_SIZE;
                    return;
                }
            }
        }

        /// @brief [[serialize]] stores an array<T1, T2> reference into a serial buffer
        /// @tparam   T: array base type
        /// @tparam   T2: array size type
        /// @param    value: value to store
        /// @param    bits: bits per value element
        template <typename T, typename T2, typename std::enable_if<
            !detail::is_format_modifier<typename serdes::array<T, T2>::elem_type>::value &&
            !detail::supported_by_bitcpy<typename serdes::array<T, T2>::elem_type>::value
            , int *>::type = nullptr>
        void store(const serdes::array<T, T2> &value, size_t bits = detail::default_bitsize<typename serdes::array<T, T2>::elem_type>::value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();

            // here the "size" reference is finally copied, in case it changed after the reference bind occurred
            size_t array_size = static_cast<size_t>(value.size);
            if (array_size > value.max_size)
            {
                array_size = value.max_size;
                status = status_e::ARRAY_SIZE_OVER_MAX;
            }
            for (size_t i = 0; i < array_size && status != status_e::NO_ERROR; i++)
                store(value.value[i], bits);
        }

        /// @brief [[serialize]] stores an array<packet_base, T2> reference into a serial buffer
        /// @tparam   T: array base type (must be derived from packet_base or have a "void format(packet&)" method)
        /// @tparam   T2: array size type
        /// @param    value: value to store
        template <typename T, typename T2, typename std::enable_if<detail::has_format_method<typename serdes::array<T, T2>::elem_type>::value, int *>::type = nullptr>
        void store(const serdes::array<T, T2> &value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();

            // here the "size" reference is finally copied, in case it changed after the reference bind occurred
            size_t array_size = static_cast<size_t>(value.size);
            if (array_size > value.max_size)
            {
                status = status_e::ARRAY_SIZE_OVER_MAX;
                return;
            }
            for (size_t i = 0; i < array_size; i++)
            {
                value.value[i].format(*this);
                if (status != status_e::NO_ERROR)
                    return;
            }
        }

        /// @brief [[serialize]] stores an array<T, T2> rvalue into a serial buffer
        /// @tparam   T: array base type
        /// @tparam   T2: array size type
        /// @param    value: value to store
        template <typename T, typename T2>
        void store(const array<T, T2> &&value)
        {
            store(value);
        }

        /// @brief [[serialize]] stores a formatter reference into a serial buffer
        /// @param    value: value to store
        template<typename T, typename std::enable_if<
            detail::is_formatter<typename detail::remove_cvref_cpp11<T>::type>::value,
            int *>::type* = nullptr>
        void store(T &&value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            if (value.formatter_lambda == nullptr)
            {
                status = status_e::FORMATTER_NOT_SET;
                return;
            }
            value.formatter_lambda(*this);
        }

        /// @brief [[serialize, pad]] applies a padding step to the store process
        /// @tparam   ST: size type of the pad
        /// @param    padding: pad value
        template <typename ST>
        void store(const serdes::pad<ST> padding)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            pad_assuming_no_prior_errors(padding.value);
        }

        /// @brief [[serialize, align]] applies an alignment step to the store process
        /// @tparam   ST: size type of the align
        /// @param    alignment: align value
        template <typename ST>
        void store(const serdes::align<ST> alignment)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            align(alignment.value);
        }

        /// @brief [[serialize]] stores a bitpack<T, ST> rvalue into a serial buffer
        /// @tparam   T: the type of the underlying bitpacked object
        /// @tparam   ST: underlying size type of the bitpack object
        /// @param    value: value to store
        template <typename T, typename ST>
        void store(const bitpack<T, ST> &&value)
        {
            store(value.value, value.bits);
        }

        /// @brief [[serialize]] stores a bitpack<T, ST> reference into a serial buffer
        /// @tparam   T: the type of the underlying bitpacked object
        /// @tparam   ST: underlying size type of the bitpack object
        /// @param    value: value to store
        template <typename T, typename ST>
        void store(const bitpack<T, ST> &value)
        {
            store(value.value, value.bits);
        }

        /// @brief [[serialize]] stores a packet_base reference into a serial buffer
        /// @param    value: value to store
        void store(const packet_base &value)
        {
            if (status != status_e::NO_ERROR)
                return;
            ensure_store();
            const_cast<packet_base &>(value).format(*this);
        }

        /// @brief [[serialize]] stores a packet_base rvalue into a serial buffer
        /// @param    value: value to store
        void store(const packet_base &&value)
        {
            store(value);
        }

        //
        // stream operator SECTION (<< is store, >> is load)
        //

        /// @brief [[deserialize]] loads serial data into the passed value (same as load)
        /// @tparam   T: value type
        /// @param    x: value reference
        /// @return   packet&: resulting modified packet process
        template <typename T>
        packet &operator>>(T &&x)
        {
            load(std::forward<T>(x));
            return *this;
        }

        /// @brief [[serialize]] stores the passed value into the serial data (same as store)
        /// @tparam   T: value type
        /// @param    x: value
        /// @return   packet&: resulting modified packet process
        template <typename T>
        packet &operator<<(T &&x)
        {
            store(std::forward<T>(x));
            return *this;
        }

        //
        // add() SECTION (same as "+", either store or load)
        //

        /// @brief adds a referenced field to the serial format for both serialization and deserialization
        /// @tparam   T: value type
        /// @param    x: value
        /// @param    bits: number of bits to store/load the value into/out-of
        template <typename T>
        void add(T &&x, size_t bits)
        {
            if (mode == mode_e::LOADING)
                load(std::forward<T>(x), bits);
            else if (mode == mode_e::STORING)
                store(std::forward<T>(x), bits);
        }

        /// @brief adds an array field to the serial format for both serialization and deserialization
        /// @tparam   T: value type
        /// @param    x: value
        /// @param    bits: number of bits to store/load the value into/out-of
        template <typename T, size_t N>
        void add(T (&x)[N], size_t bits)
        {
            if (mode == mode_e::LOADING)
                for (size_t i = 0; i < N; i++)
                    load(x[i], bits);
            else if (mode == mode_e::STORING)
                for (size_t i = 0; i < N; i++)
                    store(x[i], bits);
        }

        /// @brief adds a referenced field to the serial format for both serialization and deserialization
        /// @tparam   T: value type
        /// @param    x: value
        template <typename T, typename std::enable_if<
            !detail::is_validator<typename detail::remove_cvref_cpp11<T>::type>::value,
            int *>::type* = nullptr>
        void add(T &&x)
        {
            if (mode == mode_e::LOADING)
                load(std::forward<T>(x));
            else if (mode == mode_e::STORING)
                store(std::forward<T>(x));
        }

        /// @brief adds an array field to the serial format for both serialization and deserialization
        /// @tparam   T: value type
        /// @param    x: value
        template <typename T, size_t N>
        void add(T (&x)[N])
        {
            if (mode == mode_e::LOADING)
                for (size_t i = 0; i < N; i++)
                    load(x[i]);
            else if (mode == mode_e::STORING)
                for (size_t i = 0; i < N; i++)
                    store(x[i]);
        }

        /// @brief adds a referenced field to the serial format for both serialization and deserialization
        /// along with a validation process to run before serialization, and/or after deserialization
        /// @tparam   T: value type
        /// @param    value: value
        /// @param    validation: function that returns true if the field is valid, false to abort the process
        template <typename T, typename F>
        void add(T &&value, F &&validation)
        {
            if (status != status_e::NO_ERROR)
                return;
            if (mode == mode_e::LOADING)
                load(std::forward<T>(value));
            if (!std::forward<F>(validation)())
                status = status_e::INVALID_FIELD;
            else if (mode == mode_e::STORING)
                store(std::forward<T>(value));
        }

        /// @brief adds a referenced field to the serial format for both serialization and deserialization
        /// along with a validation process to run before serialization, and/or after deserialization
        /// via a "serdes::validate(field, validation_function)" object.
        /// Note that validate can nest other formatters, but must be the most outside specified
        /// formatter, since the validation should occur before/after the other format processes,
        /// for example:
        ///     "packet + serdes::validate(serdes::bitpack(field, 7), []{ return field > 5u; })"
        /// @tparam   T: serdes::validate type
        /// @param    value: serdes::validate object
        template <typename T, typename std::enable_if<
            detail::is_validator<typename detail::remove_cvref_cpp11<T>::type>::value,
            int *>::type* = nullptr>
        void add(T &&value)
        {
            add(value.field, value.validation);
        }

        //
        // "+" steam operator SECTION (same as add(), either store or load)
        //

#if (defined(__GNUC__) && !defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#endif
        /// @brief adds a field to the serial format for both serialization and deserialization (same as packet.add())
        /// @tparam   T: value type
        /// @param    value: value
        /// @return   packet&: modified packet process after the add
        template <typename T>
        packet &operator+(T &&value)
        {
            add(std::forward<T>(value));
            return *this;
        }
#if (defined(__GNUC__) && !defined(__clang__))
#pragma GCC diagnostic pop
#endif

        /// @brief Will return an iterator to the raw serial bytes in the packet
        /// over a selected range.
        ///
        /// An iterator is needed so that if the serial data need endianness handling
        /// you won't need to worry about it, the iterator will abstract that.
        /// @param    start: the byte index to start iterating from
        /// @param    size : the number of bytes to iterate over
        /// @return   byte_iterator_type
        inline byte_iterator_type byte_iterator(starting_byte_index start, number_of_bytes size)
        {
            if (status != status_e::NO_ERROR)
                return {buffer, 0u, 0u};
            const size_t max_num_bytes = buffer.size * buffer.element_size;
            const size_t end_byte_index_plus_one = start.value + size.value;
            if (end_byte_index_plus_one >= max_num_bytes)
            {
                status = status_e::NUM_BYTES_OVER_MAX;
                return {buffer, 0u, 0u};
            }
            return {buffer, start.value, end_byte_index_plus_one};
        }

        /// @brief Will return an iterator to the raw serial bytes in the packet
        /// starting at the specified location and ending at the current bit offset.
        ///
        /// An iterator is needed so that if the serial data need endianness handling
        /// you won't need to worry about it, the iterator will abstract that.
        /// @param    start: the byte index to start iterating from
        /// @return   byte_iterator_type
        inline byte_iterator_type previous_bytes(starting_byte_index start)
        {
            if (status != status_e::NO_ERROR)
                return {buffer, 0u, 0u};
            const size_t max_num_bytes = bit_offset/8u;
            if (start.value >= max_num_bytes)
            {
                status = status_e::START_BYTE_PAST_CURRENT;
                return {buffer, 0u, 0u};
            }
            return byte_iterator(start, number_of_bytes{max_num_bytes - start.value});
        }

        /// @brief Will return an iterator to the raw serial bytes in the packet
        /// starting at the beginning of the packet and ending at the current bit offset.
        ///
        /// An iterator is needed so that if the serial data need endianness handling
        /// you won't need to worry about it, the iterator will abstract that.
        /// @return   byte_iterator_type
        inline byte_iterator_type previous_bytes()
        {
            if (status != status_e::NO_ERROR)
                return byte_iterator(starting_byte_index{0u}, number_of_bytes{0u});
            return byte_iterator(starting_byte_index{0u}, number_of_bytes{bit_offset/8u});
        }

#if !defined(configCPP_SERDES_LIB_EXCLUDE_CPP_CRC) && BITCPY_CONSTEXPR_SUPPORTED
        /// @brief Lets you select a CRC algorithm and calculate it for the previous bytes
        /// up to the current bit offset in the packet, also optionally stores that value
        /// into a passed field if in STORING mode.
        /// @tparam   cpp_crc_type : the CRC algorithm type you want to use
        /// @param    crc_field : the field you'd like to store the value in in STORING mode
        /// @return   cpp_crc_type::type : the calculated value of the CRC
        template <typename cpp_crc_type>
        typename cpp_crc_type::type calculate_crc(typename cpp_crc_type::type *crc_field = nullptr)
        {
            auto crc_calculated = cpp_crc_type::null_crc;
            for (auto &segment : previous_bytes())
                crc_calculated = cpp_crc_type::calc(segment.bytes, segment.num_bytes, crc_calculated);
            if (crc_field != nullptr && mode == serdes::mode_e::STORING)
                *crc_field = crc_calculated;
            return crc_calculated;
        }
#endif

    private:
        /// @brief adds the specified pad bits, without any safety status checking
        /// @param    bits
        inline void pad_assuming_no_prior_errors(const size_t bits)
        {
            const size_t next_bit_offset = bit_offset + bits;
            if (next_bit_offset > bit_capacity)
            {
                status = status_e::EXCEEDED_SERIAL_SIZE;
                return;
            }
            bit_offset = next_bit_offset;
        }

        /// @brief adds the specified alignment bits, without any safety status checking
        /// @param    bits
        inline void align_assuming_no_prior_errors(size_t bits)
        {
            const size_t alignment = bit_offset % bits;
            if (alignment > 0)
            {
                const size_t next_bit_offset = bit_offset + bits - alignment;
                if (next_bit_offset > bit_capacity)
                {
                    status = status_e::EXCEEDED_SERIAL_SIZE;
                    return;
                }
                bit_offset = next_bit_offset;
            }
        }

        /// @brief ensures that the mode is in LOADING mode if the user used a load specific operator
        inline void ensure_load()
        {
            if (mode != mode_e::LOADING)
            {
                if (mode == mode_e::STORING)
                    reset();
                mode = mode_e::LOADING;
            }
        }

        /// @brief ensures that the mode is in STORING mode if the user used a store specific operator
        inline void ensure_store()
        {
            if (mode != mode_e::STORING)
            {
                if (mode == mode_e::LOADING)
                    reset();
                mode = mode_e::STORING;
            }
        }
    };

    template <typename T_array, size_t N>
    status_t packet_base::store(T_array (&target_buffer)[N], size_t max_elements, size_t bit_offset)
    {
        if (N < max_elements)
            max_elements = N;
        packet pkt_obj(serdes::sized_pointer<T_array>(&target_buffer[0], max_elements), bit_offset, mode_e::STORING);
        format(pkt_obj);
        return {pkt_obj.status, pkt_obj.bit_offset};
    }

    template <typename T_pointer, typename std::enable_if<std::is_pointer<T_pointer>::value, int *>::type>
    status_t packet_base::store(T_pointer target_buffer, size_t max_elements, size_t bit_offset)
    {
        packet pkt_obj(serdes::sized_pointer<typename std::remove_pointer<T_pointer>::type>(target_buffer, max_elements), bit_offset, mode_e::STORING);
        format(pkt_obj);
        return {pkt_obj.status, pkt_obj.bit_offset};
    }

    template <typename T_sized_pointer, typename std::enable_if<serdes::detail::is_sized_pointer<T_sized_pointer>::value, int *>::type>
    status_t packet_base::store(T_sized_pointer target_buffer, size_t bit_offset)
    {
        packet pkt_obj(target_buffer, bit_offset, mode_e::STORING);
        format(pkt_obj);
        return {pkt_obj.status, pkt_obj.bit_offset};
    }
    template <typename T_array, size_t N>
    status_t packet_base::load(const T_array (&source_buffer)[N], size_t max_elements, size_t bit_offset)
    {
        if (N < max_elements)
            max_elements = N;
        packet pkt_obj(serdes::sized_pointer<const T_array>(&source_buffer[0], N), bit_offset, mode_e::LOADING);
        format(pkt_obj);
        return {pkt_obj.status, pkt_obj.bit_offset};
    }
    template <typename T_pointer, typename std::enable_if<std::is_pointer<T_pointer>::value, int *>::type>
    status_t packet_base::load(const T_pointer source_buffer, size_t max_elements, size_t bit_offset)
    {
        packet pkt_obj(serdes::sized_pointer<const typename std::remove_pointer<T_pointer>::type>(source_buffer, max_elements), bit_offset, mode_e::LOADING);
        format(pkt_obj);
        return {pkt_obj.status, pkt_obj.bit_offset};
    }
    template <typename T_sized_pointer, typename std::enable_if<serdes::detail::is_sized_pointer<T_sized_pointer>::value, int *>::type>
    status_t packet_base::load(const T_sized_pointer target_buffer, size_t bit_offset)
    {
        packet pkt_obj(target_buffer, bit_offset, mode_e::LOADING);
        format(pkt_obj);
        return {pkt_obj.status, pkt_obj.bit_offset};
    }
    template <typename T>
    status_t packet_base::operator>>(T &&value)
    {
        return store(std::forward<T>(value));
    }
    template <typename T>
    status_t packet_base::operator<<(T &&value)
    {
        return load(std::forward<T>(value));
    }
}

#endif // _SERDES_H_
