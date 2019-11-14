////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-json](https://github.com/semenovf/pfs-json) library.
//
// Changelog:
//      2019.10.05 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <system_error>

namespace pfs {
namespace json {

////////////////////////////////////////////////////////////////////////////////
// Error codes, category, exception class
////////////////////////////////////////////////////////////////////////////////
using error_code = std::error_code;

enum class errc
{
      success = 0

// Parser errors
    , unbalanced_quote
    , bad_escaped_char
    , bad_encoded_char
    , unbalanced_array_bracket
};

class error_category : public std::error_category
{
public:
    virtual char const * name () const noexcept override
    {
        return "json_category";
    }

    virtual std::string message (int ev) const override
    {
        switch (ev) {
            case static_cast<int>(errc::success):
                return std::string{"no error"};
            case static_cast<int>(errc::unbalanced_quote):
                return std::string{"unquoted string"};
            case static_cast<int>(errc::bad_escaped_char):
                return std::string{"bad escaped char"};
            case static_cast<int>(errc::bad_encoded_char):
                return std::string{"bad encoded char"};

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

////////////////////////////////////////////////////////////////////////////////
// value: general JSON class
////////////////////////////////////////////////////////////////////////////////

enum class value_type
{
      null = 0
    , boolean
    , integer
    , real
    , string
    , object
    , array
};

class value
{
    value_type _type;
};

}} // // namespace pfs::json
