/// @file serdes_errors.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief Defines error and status types: status_e, status_t, mode_e, status2str(status_e)
///
/// @copyright (c) 2021 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef _SERDES_ERRORS_H_
#define _SERDES_ERRORS_H_

/// @brief CppSerdes library namespace
namespace serdes
{
    /// @brief error status of serialization/deserialization process
    enum class status_e
    {
        /// @brief no serialization/deserialization errors occurred
        NO_ERROR = 0,

        /// @brief during serialization/deserialization the serial array boundary was reached
        /// (causes the serdes process to abort)
        EXCEEDED_SERIAL_SIZE = 1,

        /// @brief the serdes::array object's size exceeded the maximum size of the array when evaluated
        /// (causes the serdes process to abort)
        ARRAY_SIZE_OVER_MAX = 2,

        /// @brief a fields validation check failed
        /// (causes the serdes process to abort)
        INVALID_FIELD = 3,

        /// @brief tried to loading data from a serial buffer into a temperary rvalue
        /// (causes the serdes process to abort)
        NO_LOAD_TO_RVALUE = 4,

        /// @brief the specified delimiter in a delimited_array object was not found before the end of the array
        /// (causes the serdes process to abort)
        DELIMITER_NOT_FOUND = 5,

        /// @brief a pure_virtual_formatter (a.k.a "serdes::formatter(nullptr)") was used but not overriden
        /// (causes the serdes process to abort)
        FORMATTER_NOT_SET = 6
    };

    /// @brief converts an error status enum to a c style string
    ///
    /// @param    err_status: error status enumeration
    /// @return   const char*: converted string
    inline const char *status2str(status_e err_status) noexcept
    {
        switch (err_status)
        {
        case status_e::NO_ERROR:
            return "NO_ERROR";
        case status_e::EXCEEDED_SERIAL_SIZE:
            return "EXCEEDED_SERIAL_SIZE";
        case status_e::ARRAY_SIZE_OVER_MAX:
            return "ARRAY_SIZE_OVER_MAX";
        case status_e::INVALID_FIELD:
            return "INVALID_FIELD";
        case status_e::NO_LOAD_TO_RVALUE:
            return "NO_LOAD_TO_RVALUE";
        case status_e::DELIMITER_NOT_FOUND:
            return "DELIMITER_NOT_FOUND";
        case status_e::FORMATTER_NOT_SET:
            return "FORMATTER_NOT_SET";
        default:
            return "(null)";
        }
    }

    /// @brief status returned after any serialization/deserialization process
    struct status_t
    {
        /// @brief status of the serdes (load or store) action
        status_e status;

        /// @brief number of bits processed
        size_t bits;
    };

    /// @brief the serdes mode of operation
    enum class mode_e
    {
        /// @brief (deserializing) loading from serial data into variables
        LOADING,

        /// @brief (serializing) storing variables into serial data
        STORING,

        /// @brief not yet configured for storing/loading
        UNSPECIFIED
    };
}

#endif // _SERDES_ERRORS_H_
