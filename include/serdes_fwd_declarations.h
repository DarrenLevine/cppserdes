/// @file serdes_fws_declarations.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief BEWARE here be dragons! Please include serdes.h directly if you get linking errors, this file
/// ONLY defines forward declarations for:
///
///   enum class status_e;
///   struct status_t;
///   enum class mode_e;
///   struct formatter;
///   struct packet;
///   struct packet_base;
///
/// This is useful when using object oriented design in header files that get included by a lot of other code.
/// It will get you into trouble if you try to use the methods without including serdes.h first, you will get
/// linker errors if you forget though.
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _SERDES_FWD_DECLARATIONS_H_
#define _SERDES_FWD_DECLARATIONS_H_

#include <type_traits>
#include <utility>
#include <stdint.h>

/// @brief CppSerdes library namespace
namespace serdes
{
    //
    // forward declarations
    //

    enum class status_e;
    struct status_t;
    enum class mode_e;
    struct formatter;
    struct packet;

    /// @brief inheritable base class to allow format recording and application
    /// with no additional memory storage (except for the virtual table pointer)
    struct packet_base
    {
        /// @brief override to declare the serdes process used in store() and load()
        virtual void format(packet &) = 0;

        /// @brief [[serialize]] stores data into the target "sized" serial buffer according to the format() process
        /// @tparam   T: the target buffer base type
        /// @tparam   N: the size of the buffer
        /// @param    target_buffer: the target serial data to store data into
        /// @param    bit_offset: the starting bit offset
        /// @return   serdes::status_t: the store process's resulting status
        template <typename T, size_t N>
        serdes::status_t store(T (&target_buffer)[N], size_t bit_offset = 0);

        /// @brief [[serialize]] stores data into the target (un-sized) pointer according to the format() process
        /// @tparam   T: pointer type of the target buffer
        /// @param    target_buffer: the target serial data to store data into
        /// @param    bit_offset: the starting bit offset
        /// @return   serdes::status_t: the store process's resulting status
        template <typename T, typename std::enable_if<std::is_pointer<T>::value && !std::is_array<T>::value, int *>::type = nullptr>
        serdes::status_t store(T target_buffer, size_t bit_offset = 0);

        /// @brief [[deserialize]] loads data from the source "sized" serial buffer according to the format() process
        /// @tparam   T: the source buffer base type
        /// @tparam   N: the size of the buffer
        /// @param    source_buffer: the source serial data to read data from
        /// @param    bit_offset: the starting bit offset
        /// @return   serdes::status_t: the load process's resulting status
        template <typename T, size_t N>
        serdes::status_t load(const T (&source_buffer)[N], size_t bit_offset = 0);

        /// @brief [[deserialize]] loads data into the source (un-sized) pointer according to the format() process
        /// @tparam   T: the source buffer base type
        /// @param    source_buffer: the source serial data to read data from
        /// @param    bit_offset: the starting bit offset
        /// @return   serdes::status_t: the load process's resulting status
        template <typename T>
        serdes::status_t load(const T *source_buffer, size_t bit_offset = 0);

        /// @brief [[serialize]] stores the packet_base object into the passed serial data (same as store)
        /// @tparam   T: value type
        /// @param    value: serial buffer
        /// @return   status_t: status of the store operation
        template <typename T>
        serdes::status_t operator>>(T &&value);

        /// @brief [[deserialize]] loads packet_base data into the passed serial data (same as load)
        /// @tparam   T: value type
        /// @param    value: serial buffer
        /// @return   status_t: status of the load operation
        template <typename T>
        serdes::status_t operator<<(T &&value);

        virtual ~packet_base() = default;
    };
}
#endif // _SERDES_FWD_DECLARATIONS_H_
