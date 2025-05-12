/// @file serdes_validator.h
/// @author Darren V Levine (DarrenVLevine@gmail.com)
/// @brief defines the serdes::validator class for recording arbitrary validation procedures
///
/// @copyright (c) 2025 Darren V Levine. This code is licensed under MIT license (see LICENSE file for details).
///
#ifndef SERDES_VALIDATOR_H_
#define SERDES_VALIDATOR_H_

#include "bitcpy_common.h"

/// @brief CppSerdes library namespace
namespace serdes
{
    /// @brief a field and its attached functional validation callable object,
    /// note that validation will occur after deserialization and before serialization
    /// @tparam FieldType : the field type to validate
    /// @tparam FuncType : the callable object validation routine type
    template <typename FieldType, typename FuncType>
    struct validator
    {
        FieldType field;
        FuncType validation;
        template <typename FieldTypeT, typename FuncTypeT>

        /// @brief Construct a new validator object using perfect forwarding
        /// @param    field_arg : the field to validate
        /// @param    validation_arg : the callable object validation routine
        validator(FieldTypeT &&field_arg, FuncTypeT &&validation_arg)
            : field{std::forward<FieldType>(field_arg)},
              validation{std::forward<FuncType>(validation_arg)}
        {
        }
    };

    /// @brief wraps a field and validation function together into a validator class object
    /// @tparam FieldType : the field type to validate
    /// @tparam FuncType : the callable object validation routine type
    /// @param    field_arg : the field to validate
    /// @param    validation_arg : the callable object validation routine
    /// @return   validator<FieldType, FuncType>
    template <typename FieldType, typename FuncType>
    validator<FieldType, FuncType> validate(FieldType &&field_arg, FuncType &&validation_arg)
    {
        return {std::forward<FieldType>(field_arg), std::forward<FuncType>(validation_arg)};
    }
} // namespace serdes

#endif // SERDES_VALIDATOR_H_
