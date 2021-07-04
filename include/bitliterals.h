/// @file bitliterals.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief Defines string literals for fixed sized types, useful for serdes definitions.
/// For example: Writting "0xAB_u8" would be equivalent to "static_cast<uint8_t>(0xAB)"
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _BITLITERALS_H_
#define _BITLITERALS_H_

#include <stdint.h>

/// @brief CppSerdes library namespace
namespace serdes
{
    /// @brief provides fixed size literals, useful when specifying serialization actions to convey exact bit widths
    namespace literals
    {
        /// @brief ""_zu = size_t
        inline constexpr size_t operator""_zu(unsigned long long val) noexcept { return val; }
        /// @brief ""_u8 = uint8_t
        inline constexpr uint8_t operator""_u8(unsigned long long val) noexcept { return val; }
        /// @brief ""_u16 = uint16_t
        inline constexpr uint16_t operator""_u16(unsigned long long val) noexcept { return val; }
        /// @brief ""_u32 = uint32_t
        inline constexpr uint32_t operator""_u32(unsigned long long val) noexcept { return val; }
        /// @brief ""_64 = uint64_t
        inline constexpr uint64_t operator""_u64(unsigned long long val) noexcept { return val; }
        /// @brief ""_i8 = int8_t
        inline constexpr int8_t operator""_i8(unsigned long long val) noexcept { return val; }
        /// @brief ""_i16 = int16_t
        inline constexpr int16_t operator""_i16(unsigned long long val) noexcept { return val; }
        /// @brief ""_i32 = int32_t
        inline constexpr int32_t operator""_i32(unsigned long long val) noexcept { return val; }
        /// @brief ""_i64 = int64_t
        inline constexpr int64_t operator""_i64(unsigned long long val) noexcept { return val; }
    }
}

#endif // _BITLITERALS_H_
