/// @file bitcpy_to_array.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief Defines deserialization functions in the form of:
/// size_t bitcpy(T1 &dest, const T2 source[], const size_t bit_offset, const size_t bits)
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _BITCPY_TO_ARRAY_H_
#define _BITCPY_TO_ARRAY_H_

#include "bitcpy_common.h"

/// @brief CppSerdes library namespace
namespace serdes
{
    /// @brief [[serialize, uint source]] copies the specified number of bits from a value
    /// into an array
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_unsigned_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_array *const dest, const T_val source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        constexpr size_t bits_per_T_array = sizeof(T_array) * 8u;
        constexpr size_t bits_per_T_array_minus_one = bits_per_T_array - 1u;
        size_t array_write_index = bit_offset / bits_per_T_array;
        const size_t bit_offset_from_start_index = bit_offset & bits_per_T_array_minus_one; // same as (bit_offset % sizeof(T_array)*8)

        // shortcut 1: handle exact bit size and offset match
        if (bits == bits_per_T_array && bit_offset_from_start_index == 0)
        {
            dest[array_write_index] = source;
            return bits;
        }

        // shortcut 2: if the write doesn't need to be split across array elements
        const size_t number_of_bits_after_index = bit_offset_from_start_index + bits;
        if (number_of_bits_after_index <= bits_per_T_array)
        {
            if (bits == 0) // unlikely. so we reduce the chances of needing to do this check until the last moment
                return bits;
            const T_array unaligned_mask = detail::bitmask<T_array>(bits);
            const size_t alignment_shift = bits_per_T_array - number_of_bits_after_index;
            T_array &dest_array_element = dest[array_write_index];

            // zero out intended destination bits only in the mask's range
            dest_array_element &= ~(unaligned_mask << alignment_shift);

            // fill in with the new bits from the source
            dest_array_element |= static_cast<T_array>(source & unaligned_mask) << alignment_shift;
            return bits;
        }

        // if the write DOES need to be split across > 1 array elements
        const size_t num_array_elements_touched = (number_of_bits_after_index + bits_per_T_array - 1u) / bits_per_T_array;
        const size_t bits_in_first_element = bits_per_T_array - bit_offset_from_start_index;
        size_t bits_remaining = bits - bits_in_first_element;
        {
            const T_array aligned_mask = detail::bitmask<T_array>(bits_in_first_element);
            T_array &dest_array_first_element = dest[array_write_index];
            dest_array_first_element &= ~aligned_mask;
            dest_array_first_element |= static_cast<T_array>(source >> bits_remaining) & aligned_mask;
        }
        for (size_t i = 1u; i < num_array_elements_touched - 1u; i++)
        {
            ++array_write_index;
            bits_remaining -= bits_per_T_array;
            dest[array_write_index] = static_cast<T_array>(source >> bits_remaining);
        }
        {
            ++array_write_index;
            const size_t alignment_shift = bits_per_T_array - bits_remaining;
            const T_array aligned_mask = detail::bitmask<T_array>(bits_remaining) << alignment_shift;
            T_array &dest_array_last_element = dest[array_write_index];
            dest_array_last_element &= ~aligned_mask;
            dest_array_last_element |= (static_cast<T_array>(source) << alignment_shift) & aligned_mask;
        }
        return bits;
    }

    /// @brief [[serialize, bool source]] copies the specified number of bits from a value
    /// into an array
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_bool_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_array *const dest, const T_val source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        if (bits == 1)
        {
            constexpr size_t bits_per_T_array = sizeof(T_array) * 8u;
            constexpr size_t bits_per_T_array_minus_one = bits_per_T_array - 1u;
            const T_array bit_mask = static_cast<T_array>(1u) << (bits_per_T_array_minus_one - (bit_offset & bits_per_T_array_minus_one));
            const size_t array_index = bit_offset / bits_per_T_array;
            if (source)
                dest[array_index] |= bit_mask;
            else
                dest[array_index] &= ~bit_mask;
            return bits;
        }
        const uint_fast8_t bool_as_uint = static_cast<uint_fast8_t>(source) & static_cast<uint_fast8_t>(1u);
        return bitcpy(dest, bool_as_uint, bit_offset, bits);
    }

    /// @brief [[serialize, integral value <= 8 bytes source]] copies the specified
    /// number of bits from a value into an array
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_small_non_integral_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_array *const dest, const T_val source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        return bitcpy(dest, detail::reinterpret_as_unsigned(source), bit_offset, bits);
    }

    /// @brief [[serialize, signed type source]] copies the specified number of bits
    /// from a value into an array
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_signed_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_array *const dest, const T_val source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        return bitcpy(dest, detail::reinterpret_as_unsigned(source), bit_offset, bits);
    }

    /// @brief [[serialize, partial sized_pointer source]] copies the specified number of bits
    /// from a value into an array (no default bits, since it can't necessarily be known
    /// at compile time)
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_unsigned_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_array *const dest, const sized_pointer<const T_val> &source, const size_t bit_offset, const size_t bits) noexcept
    {
        constexpr size_t bits_per_T_val = sizeof(T_val) * 8;
        const size_t total_bits_T_val = bits_per_T_val * source.size;
        if (bits == total_bits_T_val)
        {
            for (size_t i = 0; i < source.size; i++)
                bitcpy(dest, source.value[i], bit_offset + i * bits_per_T_val, bits_per_T_val);
        }
        else if (bits < total_bits_T_val)
        {
            const size_t bit_difference = total_bits_T_val - bits;
            const size_t starting_source_word = bit_difference / bits_per_T_val;
            const size_t last_bit_index = source.size - 1u;
            const size_t last_bit_suboffset = bits_per_T_val * last_bit_index - bit_difference;
            const size_t remainder_bits = bits - last_bit_suboffset;
            const std::make_signed<size_t>::type offset_per_loop = bit_offset - bit_difference;
            for (size_t i = starting_source_word; i < source.size - 1u; i++)
                bitcpy(dest, source.value[i], offset_per_loop + i * bits_per_T_val, bits_per_T_val);
            bitcpy(dest, source.value[last_bit_index], bit_offset + last_bit_suboffset, remainder_bits);
        }
        else
        {
            size_t bit_difference = bits - total_bits_T_val;
            bitcpy(dest, source.value[0], bit_offset, bits_per_T_val + bit_difference);
            bit_difference += bit_offset;
            for (size_t i = 1; i < source.size; i++)
            {
                bit_difference += bits_per_T_val;
                bitcpy(dest, source.value[i], bit_difference, bits_per_T_val);
            }
        }
        return bits;
    }

    /// @brief [[serialize, entire sized_pointer source]] copies the entire sized_pointer object
    /// from a value into an array
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_unsigned_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_array *const dest, const sized_pointer<const T_val> &source, const size_t bit_offset = 0) noexcept
    {
        return bitcpy(dest, source, bit_offset, source.bit_capacity());
    }

    /// @brief [[serialize, large non integral type source]] copies the specified number of bits
    /// from a value into an array
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_large_non_integral_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11_AND_NON_LITERAL_STORAGE size_t bitcpy(T_array *const dest, const T_val &source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        const sized_pointer<const uint8_t> source_array(reinterpret_cast<const uint8_t *const>(&source), sizeof(T_val));
        return bitcpy(dest, source_array, bit_offset, bits);
    }

    /// @brief [[serialize, size safe]] copies the specified number of bits from a value into an sized array
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array = void, typename T_val = void>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(sized_pointer<T_array> dest, const T_val &source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        if (dest.bit_capacity() < (bits + bit_offset))
            return 0;
        return bitcpy(dest.value, source, bit_offset, bits);
    }

    /// @brief [[serialize, atomic source]] copies the specified number of bits from an atomic
    /// value into an sized array
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val>
    size_t bitcpy(T_array *const dest, const std::atomic<T_val> &source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        return bitcpy(dest, source.load(), bit_offset, bits);
    }

    /// @brief [[serialize, size safe, type punned (void) dest array]] copies the specified number of bits from a
    /// value into an sized array
    ///
    /// @tparam   T_array: destination serial array base type
    /// @tparam   T_val: source value type
    /// @param    dest: pointer to the start of the destination serial array
    /// @param    source: source value reference
    /// @param    bit_offset: starting bit of the destination array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_val>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(sized_pointer<void> &dest, const T_val &source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        if (dest.bit_capacity() < (bits + bit_offset))
            return 0;
        switch (dest.element_size)
        {
        case 1:
            return bitcpy(reinterpret_cast<uint8_t *>(dest.value), source, bit_offset, bits);
        case 2:
            return bitcpy(reinterpret_cast<uint16_t *>(dest.value), source, bit_offset, bits);
        case 4:
            return bitcpy(reinterpret_cast<uint32_t *>(dest.value), source, bit_offset, bits);
        case 8:
            return bitcpy(reinterpret_cast<uint64_t *>(dest.value), source, bit_offset, bits);
        default:
            break;
        }
        return 0;
    }
}

#endif // _BITCPY_TO_ARRAY_H_
