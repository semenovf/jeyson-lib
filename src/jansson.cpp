////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////
#include "pfs/jeyson/json.hpp"
#include "pfs/jeyson/backend/jansson.hpp"
#include <jansson.h>

namespace jeyson {

template <>
json<jansson_backend>::json ()
    : _d{nullptr}
{}

template <>
json<jansson_backend>::json (std::nullptr_t)
{
    _d = json_null();
}

template <>
json<jansson_backend>::json (bool value)
{
    _d = json_boolean(value);
}

template <>
json<jansson_backend>::json (std::intmax_t value)
{
    _d = json_integer(value);
}

template <>
json<jansson_backend>::json (double value)
{
    _d = json_real(value);
}

template <>
json<jansson_backend>::json (std::string const & value)
{
    _d = json_stringn(value.c_str(), value.size());
}

template <>
json<jansson_backend>::json (char const * value)
{
    _d = json_string(value);
}

template <>
json<jansson_backend>::json (char const * value, std::size_t n)
{
    _d = json_stringn(value, n);
}

template <>
json<jansson_backend>::json (json const & other)
    : _d{nullptr}
{
    if (other._d)
        _d = json_deep_copy(other._d);
}

template <>
json<jansson_backend>::json (json && other)
{
    if (other._d) {
        _d = other._d;
        other._d = nullptr;
    }
}

template <>
json<jansson_backend>::~json ()
{
    if (_d)
        json_decref(_d);
}

template <>
json<jansson_backend> & json<jansson_backend>::operator = (json const & other)
{
    if (_d != other._d) {
        this->~json();
        _d = json_deep_copy(other._d);
    }

    return *this;
}

template <>
json<jansson_backend> & json<jansson_backend>::operator = (json && other)
{
    if (_d != other._d) {
        this->~json();

        if (other._d) {
            _d = other._d;
            other._d = nullptr;
        }
    }

    return *this;
}

template <>
json<jansson_backend>::operator bool () const noexcept
{
    return _d != nullptr;
}

template <>
bool json<jansson_backend>::is_null () const noexcept
{
    return json_is_null(_d);
}

template <>
bool json<jansson_backend>::is_bool () const noexcept
{
    return json_is_boolean(_d);
}

template <>
bool json<jansson_backend>::is_integer () const noexcept
{
    return json_is_integer(_d);
}

template <>
bool json<jansson_backend>::is_real () const noexcept
{
    return json_is_real(_d);
}

template <>
bool json<jansson_backend>::is_string () const noexcept
{
    return json_is_string(_d);
}

template <>
bool json<jansson_backend>::is_array () const noexcept
{
    return json_is_array(_d);
}

template <>
bool json<jansson_backend>::is_object () const noexcept
{
    return json_is_object(_d);
}

} // namespace jeyson
