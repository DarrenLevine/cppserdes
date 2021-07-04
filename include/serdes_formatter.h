/// @file serdes_formatter.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief defines the serdes::formatter class for recording arbitrary format lambda procedures
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _SERDES_FORMATTER_H_
#define _SERDES_FORMATTER_H_

#include <functional>
#include "bitcpy_common.h"

/// @brief CppSerdes library namespace
namespace serdes
{
    struct packet; // forward declaration

    /// @brief a lambda function wrapper that can describe any serialization/deserialization formatting
    /// process. While it does use more overhead (because of the lambda), it also can describe any
    /// format process as manipulatable runtime data, which the other hard coded interfaces don't allow.\n
    ///
    /// Examples:\n
    /// \code{.cpp}
    ///     auto arrays[2] = {serdes::init_formatter(x), serdes::init_formatter(y)};
    ///     auto lvalue = serdes::init_formatter(x);
    ///     auto rvalue = serdes::init_formatter(123);
    ///     auto complex_object = serdes::init_formatter(serdes::bitpack(y, 16));
    ///     auto complex_object = serdes::init_formatter(serdes::bitpack(y, 16));
    ///     auto value_with_validation = serdes::init_formatter(x, [&]() { return x > 7 && y > 6; });
    ///     serdes::formatter complex_custom_process = {[&](serdes::packet &p){ p + x; }};
    /// \endcode
    struct formatter
    {
        /// @brief the formatting function taking a packet process (and any captures) as instructions
        std::function<void(packet &)> formatter_lambda;
    };

    /// @brief used to initialize a formatter object to be required but uninitialized.
    /// the default behaviour if not overridden is to give a status_e::FORMATTER_NOT_SET error
    constexpr void (*pure_virtual_formatter)(packet &) = nullptr;

    /// @brief used to initialize a formatter object to be optionally overridable.
    /// the default behaviour if not overridden is to do nothing.
    inline void virtual_formatter(packet &) {}

    namespace detail
    {
        template <>
        struct default_bitsize<formatter>
        {
            static constexpr size_t value = 0u;
        };
    }

    /// @brief init_formatter() constructs a serdes::formatter object lambda without
    /// using dynamic memory allocation. A number of different use cases are supported,
    /// see serdes::formatter for examples.
    namespace init_formatter
    {
        // this namespace exists only to help user with intellisense use autocomplete
        // to find the init_formatter macro, and see the help documentation above
    }

    /// @brief init_this_formatter() constructs a serdes::formatter object lambda without
    /// using dynamic memory allocation. Same as init_formatter but captures "this", only
    /// necessary for older compilers which don't capture "this" automatically.
    namespace init_this_formatter
    {
        // this namespace exists only to help user with intellisense use autocomplete
        // to find the init_this_formatter macro, and see the help documentation above
    }
}

/// @brief init_formatter() constructs a serdes::formatter object lambda without
/// using dynamic memory allocation. A number of different use cases are supported,
/// see serdes::formatter for examples.
#define init_formatter(...)                                      \
    formatter                                                    \
    {                                                            \
        [&](serdes::packet &_p_k_t) { _p_k_t.add(__VA_ARGS__); } \
    }

/// @brief init_this_formatter() constructs a serdes::formatter object lambda without
/// using dynamic memory allocation. Same as init_formatter but captures "this", only
/// necessary for older compilers which don't capture "this" automatically.
#define init_this_formatter(...)                                       \
    formatter                                                          \
    {                                                                  \
        [&, this](serdes::packet &_p_k_t) { _p_k_t.add(__VA_ARGS__); } \
    }

#endif // _SERDES_FORMATTER_H_
