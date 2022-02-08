////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <cstdint>

struct json_t;

namespace jeyson {

struct jansson_backend
{
    using size_type       = std::size_t;
    using string_type     = std::string;
    using key_type        = std::string;

    struct rep_type {
        json_t * parent {nullptr};
        json_t * ptr {nullptr};

        rep_type () = default;
        rep_type (rep_type const & other)
            : parent(other.parent)
            , ptr(other.ptr)
        {}

        rep_type (rep_type && other)
            : parent(other.parent)
            , ptr(other.ptr)
        {
            other.parent = nullptr;
            other.ptr = nullptr;
        }

        rep_type (json_t * p, json_t * par)
            : parent(par)
            , ptr(p)
        {}
    };
};


// ////////////////////////////////////////////////////////////////////////////////
// // Element access
// ////////////////////////////////////////////////////////////////////////////////
//     /**
//      * Returns a reference to the element at specified location @a pos. In case
//      * of out of bounds result is reference to invalid value.
//      */
//     jansson_backend operator [] (size_type pos);
//
//     /**
//      * Returns a constant reference to the element at specified location @a pos.
//      * In case of out of bounds result is reference to invalid value.
//      */
//     jansson_backend const operator [] (size_type pos) const;
//
//     /**
//      * Returns a reference to the value that is mapped to a key equivalent
//      * to @a key, performing an insertion if such key does not already exist.
//      */
//     jansson_backend operator [] (key_type const & key);
//
//     /// Get boolean value
//     friend bool get_bool (jansson_backend const & ref);
//
//     /// Get integer value
//     friend std::intmax_t get_integer (jansson_backend const & ref);
//
//     /// Get real value
//     friend double get_real (jansson_backend const & ref);
//
//     /// Get string
//     friend std::string get_string (jansson_backend const & ref);
//
// };


//
// template <typename T>
// T get (jansson_backend const & ref);
//
// template <>
// inline bool get<bool> (jansson_backend const & ref)
// {
//     return get_bool(ref);
// }
//
// template <typename T>
// inline typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, T>::type
// get (jansson_backend const & ref)
// {
//     auto n = get_integer(ref);
//
//     if (n < std::numeric_limits<T>::min() || n > std::numeric_limits<T>::max())
//         JEYSON__THROW(error(errc::overflow));
//
//     return static_cast<T>(n);
// }
//
// template <typename T>
// inline typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, T>::type
// get (jansson_backend const & ref)
// {
//     auto n = static_cast<std::uintmax_t>(get_integer(ref));
//
//     if (n > std::numeric_limits<T>::max())
//         JEYSON__THROW(error(errc::overflow));
//
//     return static_cast<T>(n);
// }
//
// template <>
// inline double get<double> (jansson_backend const & ref)
// {
//     return get_real(ref);
// }
//
// template <>
// inline float get<float> (jansson_backend const & ref)
// {
//     auto f = get_real(ref);
//
//     if (f < std::numeric_limits<float>::min() || f > std::numeric_limits<float>::max())
//         JEYSON__THROW(error(errc::overflow));
//
//     return static_cast<float>(f);
// }
//
// template <>
// inline std::string get<std::string> (jansson_backend const & ref)
// {
//     return get_string(ref);
// }

} // namespace jeyson
