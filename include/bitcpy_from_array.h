/// @file bitcpy_from_array.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief Defines serialization functions in the form of:
/// size_t bitcpy(T1 dest[], const T2 source, const size_t bit_offset, const size_t bits)
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _BITCPY_FROM_ARRAY_H_
#define _BITCPY_FROM_ARRAY_H_

#include "bitcpy_common.h"

/// @brief CppSerdes library namespace
namespace serdes
{

    /// @brief [[deserialize, uint destination]] copies the specified number of bits from an array
    /// into a value
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_unsigned_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_val &dest, const T_array *const source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        constexpr size_t bits_per_T_array = sizeof(T_array) * 8u;
        constexpr size_t bits_per_T_array_minus_one = bits_per_T_array - 1u;
        size_t array_read_index = bit_offset / bits_per_T_array;
        const size_t bit_offset_from_start_index = bit_offset & bits_per_T_array_minus_one; // same as (bit_offset % sizeof(T_array)*8)

        // shortcut 1: handle exact bit size and offset match
        if (bits == bits_per_T_array && bit_offset_from_start_index == 0)
        {
            dest = source[array_read_index];
            return bits;
        }

        // shortcut 2: if the write doesn't need to be split across array elements
        const size_t number_of_bits_after_index = bit_offset_from_start_index + bits;
        if (number_of_bits_after_index <= bits_per_T_array)
        {
            if (bits == 0) // unlikely. so we reduce the chances of needing to do this check until the last moment
                return bits;
            const T_array unaligned_mask = detail::bitmask<T_val>(bits);
            const size_t alignment_shift = bits_per_T_array - number_of_bits_after_index;
            dest = static_cast<T_array>(source[array_read_index] >> alignment_shift) & unaligned_mask;
            return bits;
        }

        // shortcut 3: if there is no left 0 padding, and we wouldn't exceed the buffer
        // safety limit by possibly reading past the end of the array, we can avoid the loop
        // by using big_endian_memcpy
        constexpr size_t bits_per_T_val = sizeof(T_val) * 8u;
        if (number_of_bits_after_index <= bits_per_T_val)
        {
            const size_t alignment_shift = bits_per_T_val - number_of_bits_after_index;
            if (alignment_shift < bits_per_T_array)
            {
                dest = detail::big_endian_memcpy<T_val, T_array>(&source[array_read_index]);
                const T_val unaligned_mask = detail::bitmask<T_val>(bits);
                dest = static_cast<T_val>(dest >> alignment_shift) & unaligned_mask;
                return bits;
            }
        }

        // if the read DOES need to be split across > 1 array elements, or left 0 padding is needed
        const size_t num_array_elements_touched_minus_one = (number_of_bits_after_index + bits_per_T_array - 1u) / bits_per_T_array - 1u;
        const size_t bits_in_first_element = bits_per_T_array - bit_offset_from_start_index;
        size_t bits_remaining = bits - bits_in_first_element;
        size_t start_index = 1u;
        if (bits_remaining < bits_per_T_val)
        {
            const T_array unaligned_mask = detail::bitmask<T_array>(bits_in_first_element);
            dest = static_cast<T_val>(source[array_read_index] & unaligned_mask) << bits_remaining;
        }
        else
        {
            // in the case of left 0 padding, don't manually go through the for loop, just skip ahead to where
            // in the loop it would start writting meaningful data
            dest = 0;
            const size_t start_index_minus_one = (bits_remaining - bits_per_T_val) / bits_per_T_array;
            start_index = start_index_minus_one + 1u;
            bits_remaining -= bits_per_T_array * start_index_minus_one;
            array_read_index += start_index_minus_one;
        }
        for (size_t i = start_index; i < num_array_elements_touched_minus_one; i++)
        {
            bits_remaining -= bits_per_T_array;
            ++array_read_index;
            dest |= static_cast<T_val>(source[array_read_index]) << bits_remaining;
        }
        {
            const size_t remainder_right_shift = bits_per_T_array - bits_remaining;
            const T_array unaligned_mask = detail::bitmask<T_array>(bits_remaining);
            ++array_read_index;
            dest |= static_cast<T_val>((source[array_read_index] >> remainder_right_shift) & unaligned_mask);
        }
        return bits;
    }

    /// @brief [[deserialize, bool destination]] copies the specified number of bits from an array into
    /// a boolean value
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type, a boolean for this case.
    /// @param    dest: destination boolean value reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_bool_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_val &dest, const T_array *const source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        if (bits == 1)
        {
            constexpr size_t bits_per_T_array = sizeof(T_array) * 8u;
            constexpr size_t bits_per_T_array_minus_one = bits_per_T_array - 1u;
            const T_array bit_mask = static_cast<T_array>(1u) << (bits_per_T_array_minus_one - (bit_offset & bits_per_T_array_minus_one));
            const size_t array_index = bit_offset / bits_per_T_array;
            dest = (source[array_index] & bit_mask) == bit_mask;
            return bits;
        }
        uint8_t temp_write_val = 0u;
        bitcpy(temp_write_val, source, bit_offset, bits);
        dest = temp_write_val != 0u;
        return bits;
    }

    /// @brief [[deserialize, integral value <= 8 bytes destination]] copies the specified number of bits
    /// from an array into a integral value with <= 8 bytes
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_small_non_integral_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_val &dest, const T_array *const source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        return bitcpy(detail::reinterpret_as_unsigned(dest), source, bit_offset, bits);
    }

    /// @brief [[deserialize, signed destination]] copies the specified number of bits from an array into a
    /// signed value
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_signed_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_val &dest, const T_array *const source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        bitcpy(detail::reinterpret_as_unsigned(dest), source, bit_offset, bits);
        detail::extend_sign(dest, bits);
        return bits;
    }

    /// @brief [[deserialize, partial sized_pointer destination]] copies the specified number of bits from an array into a
    /// sized_pointer array (no default bits, since it can't necessarily be known at compile time)
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value sized_pointer<T_val> reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_unsigned_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(sized_pointer<T_val> &dest, const T_array *const source, const size_t bit_offset, const size_t bits) noexcept
    {
        constexpr size_t bits_per_T_val = sizeof(T_val) * 8;
        const size_t total_bits_T_val = bits_per_T_val * dest.size;
        if (bits == total_bits_T_val)
        {
            for (size_t i = 0; i < dest.size; i++)
                bitcpy(dest.value[i], source, bit_offset + i * bits_per_T_val, bits_per_T_val);
        }
        else if (bits < total_bits_T_val)
        {
            const size_t bit_difference = total_bits_T_val - bits;
            const size_t starting_source_word = bit_difference / bits_per_T_val;
            const size_t last_bit_index = dest.size - 1u;
            const size_t last_bit_suboffset = bits_per_T_val * last_bit_index - bit_difference;
            const size_t remainder_bits = bits - last_bit_suboffset;
            const std::make_signed<size_t>::type offset_per_loop = bit_offset - bit_difference;
            for (size_t i = starting_source_word; i < dest.size - 1u; i++)
                bitcpy(dest.value[i], source, offset_per_loop + i * bits_per_T_val, bits_per_T_val);
            bitcpy(dest.value[last_bit_index], source, bit_offset + last_bit_suboffset, remainder_bits);
        }
        else
        {
            size_t adjusted_offset = bit_offset + bits - total_bits_T_val;
            bitcpy(dest.value[0], source, adjusted_offset, bits_per_T_val);
            for (size_t i = 1; i < dest.size; i++)
            {
                adjusted_offset += bits_per_T_val;
                bitcpy(dest.value[i], source, adjusted_offset, bits_per_T_val);
            }
        }
        return bits;
    }

    /// @brief [[deserialize, entire sized_pointer destination]] copies the specified number of bits from
    /// an array into a sized_pointer array using a bit length equal to the entire bit capacity
    /// of the sized_pointer object.
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value sized_pointer<T_val> reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_unsigned_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(sized_pointer<T_val> &dest, const T_array *const source, const size_t bit_offset = 0) noexcept
    {
        return bitcpy(dest, source, bit_offset, dest.bit_capacity());
    }

    /// @brief [[deserialize, large non integral type dest]] copies the specified number of bits from
    /// an array into a large non integral type value
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val, detail::requires_large_non_integral_type<T_val> * = nullptr>
#ifndef __clang__
    CONSTEXPR_ABOVE_CPP11
#endif
        size_t
        bitcpy(T_val &dest, const T_array *const source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        sized_pointer<uint8_t> dest_array(reinterpret_cast<uint8_t *>(&dest), sizeof(T_val));
        return bitcpy(dest_array, source, bit_offset, bits);
    }

    /// @brief [[deserialize, size safe]] copies the specified number of bits from an sized_pointer into a
    /// value. used to prevent reading beyond the boundary of the source array.
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array = void, typename T_val = void, detail::requires_not_a_pointer_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_val &dest, const sized_pointer<T_array> source, size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        if (source.bit_capacity() < (bits + bit_offset))
            return 0;
        bitcpy(dest, source.value, bit_offset, bits);
        return bits;
    }

    // either the from_array or the to_array implimentation needs a specialization to de-conflict
    // pointer to pointer transfers the from_array was arbitrarily picked, the result is that for
    // the ambiguous case, they can either casting to a uintptr_t. Or they can use sized_pointer
    // to indicate the array object, like so:
    //
    // serdes::bitcpy(pointer_value, serdes::sized_pointer(buffer)); // from array
    // serdes::bitcpy(serdes::sized_pointer(buffer), pointer_value); // to array
    namespace detail
    {
        template <typename T_array, typename T_val>
        CONSTEXPR_ABOVE_CPP11 size_t bitcpy_disambiguous_pointer_from_array(T_val &dest, const T_array *const source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
        {
            return bitcpy(detail::reinterpret_as_unsigned(dest), source, bit_offset, bits);
        }
    }

    /// @brief [[deserialize, size safe, pointer dest]] copies the specified number of bits from an
    /// sized_pointer into a value. used to prevent reading beyond the boundary of the
    /// source array.
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array = void, typename T_val = void, detail::requires_pointer_type<T_val> * = nullptr>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_val &dest, const sized_pointer<T_array> source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        if (source.bit_capacity() < (bits + bit_offset))
            return 0;
        return detail::bitcpy_disambiguous_pointer_from_array(dest, source.value, bit_offset, bits);
    }

    /// @brief [[deserialize, atomic dest]] copies the specified number of bits from an array
    /// into an atomic value
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_array, typename T_val>
    size_t bitcpy(std::atomic<T_val> &dest, const T_array *const source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        T_val temp_value = 0;
        const size_t bits_written = bitcpy(temp_value, source, bit_offset, bits);
        if (bits_written > 0)
            dest.store(temp_value);
        return bits_written;
    }

    /// @brief [[deserialize, size safe, type punned (void) source array]] copies the specified number of bits
    /// from an array with a runtime determined base type into a value
    ///
    /// @tparam   T_array: serial array base type
    /// @tparam   T_val: destination value type
    /// @param    dest: destination value reference
    /// @param    source: pointer to the start of the source serial array
    /// @param    bit_offset: starting bit of the source array to start copying from
    /// @param    bits: number of bits to copy from
    /// @return   size_t: number of bits coppied
    template <typename T_val = void>
    CONSTEXPR_ABOVE_CPP11 size_t bitcpy(T_val &dest, const sized_pointer<void> &source, const size_t bit_offset = 0, const size_t bits = detail::default_bitsize<T_val>::value) noexcept
    {
        if (source.bit_capacity() < (bits + bit_offset))
            return 0;
        switch (source.element_size)
        {
        case 1:
            return bitcpy(dest, reinterpret_cast<const uint8_t *>(source.value), bit_offset, bits);
        case 2:
            return bitcpy(dest, reinterpret_cast<const uint16_t *>(source.value), bit_offset, bits);
        case 4:
            return bitcpy(dest, reinterpret_cast<const uint32_t *>(source.value), bit_offset, bits);
        case 8:
            return bitcpy(dest, reinterpret_cast<const uint64_t *>(source.value), bit_offset, bits);
        default:
            break;
        }
        return 0;
    }
}

#endif // _BITCPY_FROM_ARRAY_H_
