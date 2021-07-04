/// @file bitcpy.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief Includes all the bitcpy definitions for serialization and deserialization
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _BITCPY_H_
#define _BITCPY_H_

// size_t bitcpy(T1 &dest, const T2 source[], const size_t bit_offset, const size_t bits)
#include "bitcpy_to_array.h"

// size_t bitcpy(T1 dest[], const T2 source, const size_t bit_offset, const size_t bits)
#include "bitcpy_from_array.h"

#endif // _BITCPY_H_
