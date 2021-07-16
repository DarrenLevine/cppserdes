/// @file bitcpy_sized_pointer.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief Defines sized_pointer, similar to std::array but with with size as a runtime constant
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _BITCPY_SIZED_POINTER_H_
#define _BITCPY_SIZED_POINTER_H_

#include <stdint.h>
#include <type_traits>

/// @brief CppSerdes library namespace
namespace serdes
{
    /// @brief Holds a pointer to an array (with its type information) with a constant size
    /// @tparam   T_array: the element type for the pointer
    template <typename T_array>
    struct sized_pointer
    {
        /// @brief the underlying pointer
        T_array *const value;

        /// @brief size of the array
        const size_t size;

        /// @brief Construct a new sized pointer object from a raw pointer and a passed size
        /// @param    v : raw pointer to the head of the array
        /// @param    N : size of the array
        sized_pointer(T_array *const v, const size_t N) noexcept : value{v}, size{N} {}

        /// @brief Construct a new sized pointer object from the array itself
        /// @tparam   N : size of the array
        /// @param    v : pointer to the head of the array
        template <size_t N>
        sized_pointer(T_array (&v)[N]) noexcept : value{v}, size{N} {}

        /// @brief returns the number of bits in the array
        size_t bit_capacity() const noexcept
        {
            return size * sizeof(T_array) * 8u;
        }
    };

    /// @brief Holds a pointer to a void array (with type information stored
    /// as a runtime element size paramenter) with a constant size
    template <>
    struct sized_pointer<void>
    {
        /// @brief the underlying pointer reinterpreted as a void* type
        void *const value;

        /// @brief size of the array
        const size_t size;

        /// @brief Number of bytes per element of the original array type, same as sizeof(T_original)
        const uint_fast8_t element_size;

        /// @brief Construct a new sized pointer object from a raw pointer and a passed size
        /// @tparam   T: original type of pointer
        /// @param    v: pointer to the head of the array
        /// @param    N: size of the array
        template <typename T>
        sized_pointer(T *const v, const size_t N) noexcept : value{const_cast<void *const>(reinterpret_cast<volatile const void *const>(v))}, size{N}, element_size{sizeof(T)} {}

        /// @brief Construct a new sized pointer object from the array itself
        /// @tparam   T: original type of pointer
        /// @tparam   N: size of the array
        /// @param    v: pointer to the head of the array
        template <typename T, size_t N>
        sized_pointer(T (&v)[N]) noexcept : value{const_cast<void *const>(reinterpret_cast<volatile const void *const>(v))}, size{N}, element_size{sizeof(T)}
        {
        }

        /// @brief returns the number of bits in the array
        size_t bit_capacity() const noexcept
        {
            return size * element_size * 8u;
        }
    };

    // implimentation details
    namespace detail
    {
        template <typename>
        struct is_sized_pointer : std::false_type
        {
        };

        template <typename T>
        struct is_sized_pointer<sized_pointer<T>> : std::true_type
        {
        };
    }
}

#endif // _BITCPY_SIZED_POINTER_H_
