/// @file serdes_byte_iterator.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief Defines a iterator for scanning through the bytes of a serdes::packet even when that packet is
/// not composed of bytes, but instead based on a uint16_t, uint32_t, or uint64_t array.
/// This is particularly useful when dealing with checksum APIs, where the API may require bytes as an input,
/// but you might not want to deal with flipping bytes due to endianness when dealing with uint32_t[] arrays
/// for example.
///
/// @copyright (c) 2025 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef SERDES_BYTE_ITERATOR_H_
#define SERDES_BYTE_ITERATOR_H_

#include "bitcpy_common.h"

/// @brief CppSerdes library namespace
namespace serdes
{
    /// @brief allows iteration through the bytes of a buffer, even when the buffer's
    /// underlying type is not a byte array, but instead something larger like a uin32_t[].
    class byte_iterator_type
    {
    public:
        /// @brief provides a segment of 1 multiple bytes
        /// for packets that abstract an actual byte array, this can refer to the entire packet
        struct byte_segment
        {
            uint8_t *bytes   = nullptr;
            size_t num_bytes = 0u;
        };

        /// @brief iterator that scans through the bytes, either one at a time
        /// when dealing with endianness details, or all at once if possible (bug endian
        /// machines or when the source data was an actual byte array).
        class iterator
        {
        public:
            inline explicit iterator(byte_iterator_type *parg) : p_parent(parg) {}
            inline iterator &operator++()
            {
                if (p_parent != nullptr)
                {
                    // for element_size == 1 or for big endian machines, the data is already in
                    // correctly ordered byte form, so it doesn't need to be iterated over
                    // and we can exit here immediately by setting p_parent == nullptr.
                    if (p_parent->buffer.element_size == 1 || !detail::on_little_endian_platform())
                    {
                        p_parent = nullptr;
                    }
                    // for all other data, it needs to be iterated over to correct for endianness
                    else if (++p_parent->start_index && p_parent->start_index >= p_parent->end_plus_one_index)
                    {
                        p_parent = nullptr;
                    }
                }
                return *this;
            }
            inline bool operator!=(const iterator &) const
            {
                return p_parent != nullptr;
            }
            byte_segment &operator*()
            {
                if (p_parent == nullptr || p_parent->start_index >= p_parent->end_plus_one_index)
                {
                    current_segment.bytes     = nullptr;
                    current_segment.num_bytes = 0u;
                    return current_segment;
                }
                auto &p       = *p_parent;
                uint8_t *data = reinterpret_cast<uint8_t *>(p.buffer.value);
                if (detail::on_little_endian_platform())
                {
                    const size_t elem_sz = p.buffer.element_size;
                    if (elem_sz == 1u)
                    {
                        current_segment.bytes     = &data[p.start_index];
                        current_segment.num_bytes = p.end_plus_one_index - p.start_index;
                    }
                    else if (elem_sz == 2u || elem_sz == 4u || elem_sz == 8u)
                    {
                        // this equation is the same as:
                        //   "start_index + elem_sz - (start_index % elem_sz)*2 - 1u;"
                        // when elem_sz is power of 2
                        // used to find the byte offset that corrects for endianness flips for various element sizes
                        const size_t byte_offset  = p.start_index + elem_sz - ((p.start_index & (elem_sz - 1u)) << 1) - 1u;
                        current_segment.bytes     = &data[byte_offset];
                        current_segment.num_bytes = 1u;
                    }
                    else
                    {
                        current_segment.bytes     = nullptr;
                        current_segment.num_bytes = 0u;
                    }
                }
                else
                {
                    current_segment.bytes     = &data[p.start_index];
                    current_segment.num_bytes = p.end_plus_one_index - p.start_index;
                }
                return current_segment;
            }

        private:
            byte_iterator_type *p_parent;
            byte_segment current_segment{};
        };
        inline iterator begin() { return iterator(this); }
        inline iterator end() { return iterator(nullptr); }

    private:
        friend struct packet;
        sized_pointer<void> &buffer;
        size_t start_index;
        size_t end_plus_one_index;
        inline byte_iterator_type(
            sized_pointer<void> &data_arg,
            size_t starting_byte,
            size_t ending_byte)
            : buffer{data_arg},
              start_index{starting_byte},
              end_plus_one_index{ending_byte}
        {
        }
    };

    /// @brief explict type for representing a starting byte index
    /// (since it's easy to mess up the iterator inputs otherwise)
    struct starting_byte_index
    {
        size_t value = 0u;
        explicit starting_byte_index(size_t val) : value{val} {}
    };

    /// @brief explict type for representing the number of bytes to iterate over
    /// (since it's easy to mess up the iterator inputs otherwise)
    struct number_of_bytes
    {
        size_t value = 0u;
        explicit number_of_bytes(size_t val) : value{val} {}
    };
} // namespace serdes

#endif // SERDES_BYTE_ITERATOR_H_