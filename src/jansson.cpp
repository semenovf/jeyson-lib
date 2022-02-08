////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////
#include "pfs/jeyson/json.hpp"
#include "pfs/jeyson/error.hpp"
#include "pfs/jeyson/backend/jansson.hpp"
#include <jansson.h>
#include <cassert>

namespace jeyson {


// ////////////////////////////////////////////////////////////////////////////////
// // Element access
// ////////////////////////////////////////////////////////////////////////////////
//
// jansson_backend::basic_json
// jansson_backend::basic_json::operator [] (size_type pos)
// {
//     if (!_ptr)
//         return basic_json{_ptr, _parent};
//
//     if (!json_is_array(_ptr))
//         return basic_json{_ptr, _parent};
//
//     auto ptr = json_array_get(_ptr, pos);
//
//     if (!ptr)
//         return basic_json{_ptr, _parent};
//
//     return basic_json{ptr, _ptr};
// }
//
// jansson_backend::basic_json
// jansson_backend::basic_json::operator [] (key_type const & key)
// {
//     if (!_ptr)
//         return basic_json{_ptr, _parent};
//
//     if (! json_is_object(_ptr))
//         return basic_json{_ptr, _parent};
//
//     auto ptr = json_object_getn(_ptr, key.c_str(), key.size());
//
//     if (!ptr) {
//         auto rc = json_object_setn_nocheck(_ptr, key.c_str(), key.size(), json_null());
//
//         if (rc != 0)
//             return basic_json{_ptr, _parent};
//     }
//
//     ptr = json_object_getn(_ptr, key.c_str(), key.size());
//
//     assert(ptr);
//
//     return basic_json{ptr, _ptr};
// }
//
// bool get_bool (jansson_backend::basic_json const & ref)
// {
//     if (!ref._ptr)
//         JEYSON__THROW(error(errc::incopatible_type));
//
//     if (!json_is_boolean(ref._ptr))
//         JEYSON__THROW(error(errc::incopatible_type));
//
//     return json_is_true(ref._ptr) != 0;
// }
//
// std::intmax_t get_integer (jansson_backend::basic_json const & ref)
// {
//     if (!ref._ptr)
//         JEYSON__THROW(error(errc::incopatible_type));
//
//     if (!json_is_integer(ref._ptr))
//         JEYSON__THROW(error(errc::incopatible_type));
//
//     return json_integer_value(ref._ptr);
// }
//
// double get_real (jansson_backend::basic_json const & ref)
// {
//     if (!ref._ptr)
//         JEYSON__THROW(error(errc::incopatible_type));
//
//     if (!json_is_real(ref._ptr))
//         JEYSON__THROW(error(errc::incopatible_type));
//
//     return json_real_value(ref._ptr);
// }
//
// std::string get_string (jansson_backend::basic_json const & ref)
// {
//     if (!ref._ptr)
//         JEYSON__THROW(error(errc::incopatible_type));
//
//     if (!json_is_string(ref._ptr))
//         JEYSON__THROW(error(errc::incopatible_type));
//
//     return std::string(json_string_value(ref._ptr)
//         , json_string_length(ref._ptr));
// }
//
// ////////////////////////////////////////////////////////////////////////////////
// // json
// ////////////////////////////////////////////////////////////////////////////////
//
// // template <>
// // json<jansson_backend>::reference
// // json<jansson_backend>::operator [] (size_type pos)
// // {
// //     return json<jansson_backend>::reference{_d, nullptr}[pos];
// // }
// //
// // template <>
// // json<jansson_backend>::const_reference
// // json<jansson_backend>::operator [] (size_type pos) const
// // {
// //     return json<jansson_backend>::reference{_d, nullptr}[pos];
// // }
// //
// // template <>
// // json<jansson_backend>::reference
// // json<jansson_backend>::operator [] (key_type const & key)
// // {
// //     return json<jansson_backend>::reference{_d, nullptr}[key];
// // }

////////////////////////////////////////////////////////////////////////////////
// Constructors, destructors, assignment operators
////////////////////////////////////////////////////////////////////////////////
template <>
json<jansson_backend>::json (jansson_backend::rep_type const & rep)
    : _d(rep)
{}

template <>
json<jansson_backend>::json (jansson_backend::rep_type && rep)
    : _d(std::move(rep))
{}

template <>
json<jansson_backend>::json ()
    : _d()
{}

template <>
json<jansson_backend>::json (std::nullptr_t)
    : _d(json_null(), nullptr)
{}

template <>
json<jansson_backend>::json (bool value)
    : _d(json_boolean(value), nullptr)
{}

template <>
json<jansson_backend>::json (std::intmax_t value)
    : _d(json_integer(value), nullptr)
{}

template <>
json<jansson_backend>::json (double value)
    : _d(json_real(value), nullptr)
{}

template <>
json<jansson_backend>::json (std::string const & value)
    : _d(json_stringn(value.c_str(), value.size()), nullptr)
{}

template <>
json<jansson_backend>::json (char const * value)
    : _d(json_string(value), nullptr)
{}

template <>
json<jansson_backend>::json (char const * value, std::size_t n)
    : _d(json_stringn_nocheck(value, n), nullptr)
{}

template <>
json<jansson_backend>::json (json const & other)
    : _d()
{
    if (other._d.ptr)
        _d.ptr = json_deep_copy(other._d.ptr);
}

template <>
json<jansson_backend>::json (json && other)
{
    if (other._d.ptr) {
        _d.ptr = other._d.ptr;
        other._d.ptr = nullptr;
    }
}

template <>
json<jansson_backend>::~json ()
{
    if (_d.ptr)
        json_decref(_d.ptr);
}

template <>
json<jansson_backend> & json<jansson_backend>::operator = (json const & other)
{
    if (_d.ptr != other._d.ptr) {
        this->~json();
        _d.ptr = json_deep_copy(other._d.ptr);
    }

    return *this;
}

template <>
json<jansson_backend> & json<jansson_backend>::operator = (json && other)
{
    if (_d.ptr != other._d.ptr) {
        this->~json();

        if (other._d.ptr) {
            _d.ptr = other._d.ptr;
            other._d.ptr = nullptr;
        }
    }

    return *this;
}

template <>
inline json<jansson_backend>::operator bool () const noexcept
{
    return _d.ptr != nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Type quieries
////////////////////////////////////////////////////////////////////////////////

template <>
bool is_null (json<jansson_backend> const & j)
{
    return j._d.ptr ? json_is_null(j._d.ptr) : false;
}

template <>
bool is_bool (json<jansson_backend> const & j)
{
    return j._d.ptr ? json_is_boolean(j._d.ptr) : false;
}

template <>
bool is_integer (json<jansson_backend> const & j)
{
    return j._d.ptr ? json_is_integer(j._d.ptr) : false;
}

template <>
bool is_real (json<jansson_backend> const & j)
{
    return j._d.ptr ? json_is_real(j._d.ptr) : false;
}

template <>
bool is_string (json<jansson_backend> const & j)
{
    return j._d.ptr ? json_is_string(j._d.ptr) : false;
}

template <>
bool is_array (json<jansson_backend> const & j)
{
    return j._d.ptr ? json_is_array(j._d.ptr) : false;
}

template <>
bool is_object (json<jansson_backend> const & j)
{
    return j._d.ptr ? json_is_object(j._d.ptr) : false;
}

////////////////////////////////////////////////////////////////////////////////
// Capacity
////////////////////////////////////////////////////////////////////////////////
template <>
json<jansson_backend>::size_type
json<jansson_backend>::size () const noexcept
{
    if (!_d.ptr)
        return 0;

    if (is_object(*this))
        return json_object_size(_d.ptr);

    if (is_array(*this))
        return json_array_size(_d.ptr);

    // Scalar types
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Modifiers
////////////////////////////////////////////////////////////////////////////////
template <>
void
json<jansson_backend>::push_back (json && value)
{
    if (!value)
        JEYSON__THROW(error(errc::invalid_argument));

    if (!_d.ptr)
        _d.ptr = json_array();

    if (!is_array(*this))
        JEYSON__THROW(error(errc::incopatible_type));

    auto rc = json_array_append_new(_d.ptr, value._d.ptr);

    assert(rc == 0);

    value._d.ptr = nullptr;
}

template <>
void
json<jansson_backend>::push_back (json const & value)
{
    if (!value)
        JEYSON__THROW(error(errc::invalid_argument));

    if (!_d.ptr)
        _d.ptr = json_array();

    if (!is_array(*this))
        JEYSON__THROW(error(errc::incopatible_type));

    json j{json_deep_copy(value._d.ptr)};

    this->push_back(std::move(j));
}

////////////////////////////////////////////////////////////////////////////////
// Comparison operators
////////////////////////////////////////////////////////////////////////////////

template <>
bool
operator == (json<jansson_backend> const & lhs, json<jansson_backend> const & rhs)
{
    if (!lhs._d.ptr && !rhs._d.ptr)
        return true;

    return json_equal(lhs._d.ptr, rhs._d.ptr) == 1;
}

} // namespace jeyson
