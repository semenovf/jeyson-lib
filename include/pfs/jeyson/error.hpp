////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2019.12.05 Initial version (pfs-json).
//      2022.02.07 Initial version (jeyson-lib).
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "pfs/error.hpp"

namespace jeyson {

enum class errc
{
      success = 0

// Parser errors
//     , forbidden_root_element
//     , unbalanced_quote
//     , bad_escaped_char
//     , bad_encoded_char
//     , unbalanced_array_bracket
//     , unbalanced_object_bracket
//     , bad_member_name
//     , bad_json_sequence

    , incopatible_type
};

class error_category : public std::error_category
{
public:
    virtual char const * name () const noexcept override
    {
        return "jeyson::error_category";
    }

    virtual std::string message (int ev) const override
    {
        switch (ev) {
            case static_cast<int>(errc::success):
                return std::string{"no error"};
//             case static_cast<int>(errc::forbidden_root_element):
//                 return std::string{"root element is forbidden"};
//             case static_cast<int>(errc::unbalanced_quote):
//                 return std::string{"unquoted string"};
//             case static_cast<int>(errc::bad_escaped_char):
//                 return std::string{"bad escaped char"};
//             case static_cast<int>(errc::bad_encoded_char):
//                 return std::string{"bad encoded char"};
//             case static_cast<int>(errc::unbalanced_array_bracket):
//                 return std::string{"unbalanced array bracket"};
//             case static_cast<int>(errc::unbalanced_object_bracket):
//                 return std::string{"unbalanced object bracket"};
//             case static_cast<int>(errc::bad_member_name):
//                 return std::string{"bad member name"};
//             case static_cast<int>(errc::bad_json_sequence):
//                 return std::string{"bad json sequence"};

            case static_cast<int>(errc::incopatible_type):
                return std::string{"incopatible type"};

            default: return std::string{"unknown JSON error"};
        }
    }
};

inline std::error_category const & get_error_category ()
{
    static error_category instance;
    return instance;
}

inline std::error_code make_error_code (errc e)
{
    return std::error_code(static_cast<int>(e), get_error_category());
}

class error: public pfs::error
{
public:
    using pfs::error::error;
};

} // namespace jeyson
