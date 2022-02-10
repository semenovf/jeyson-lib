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
#include "pfs/fmt.hpp"
#include <jansson.h>
#include <limits>
#include <cassert>

namespace jeyson {

static_assert(std::numeric_limits<std::intmax_t>::max()
    == std::numeric_limits<json_int_t>::max()
    , "");

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
        _d.parent = nullptr;
        other._d.ptr = nullptr;
    }
}

template <>
json<jansson_backend>::~json ()
{
    // !_d.parent -> implies a reference
    if (_d.ptr && !_d.parent)
        json_decref(_d.ptr);
}

template <>
json<jansson_backend> & json<jansson_backend>::operator = (json const & other)
{
    if (this != & other) {
        if (_d.ptr != other._d.ptr) {
            this->~json();
            _d.ptr = json_deep_copy(other._d.ptr);
        }
    }

    return *this;
}

template <>
json<jansson_backend> & json<jansson_backend>::operator = (json && other)
{
    if (this != & other) {
        if (_d.ptr != other._d.ptr) {
            this->~json();

            if (other._d.ptr) {
                _d.ptr = other._d.ptr;
                _d.parent = nullptr;
                other._d.ptr = nullptr;
            }
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
// Element access
////////////////////////////////////////////////////////////////////////////////

static json_t * array_elem_checked (json_t * arr, std::size_t pos)
{
    if (!arr)
        return nullptr;

    if (!json_is_array(arr))
        return nullptr;

    auto ptr = json_array_get(arr, pos);

    if (!ptr)
        return nullptr;

    return ptr;
}

static json_t * object_elem_checked (json_t * obj
    , char const * key
    , std::size_t length
    , bool insert_in_absence)
{
    if (!obj)
        return nullptr;

    if (!json_is_object(obj))
        return nullptr;

    auto ptr = json_object_getn(obj, key, length);

    // Not found, insert new `null` element if instructed it by `insert` flag
    if (!ptr) {
        if (insert_in_absence) {
            auto rc = json_object_setn_nocheck(obj, key, length, json_null());

            if (rc != 0)
                return nullptr;
        } else {
            return nullptr;
        }
    }

    ptr = json_object_getn(obj, key, length);

    assert(ptr);

    return ptr;
}

template <>
json<jansson_backend>::reference
json<jansson_backend>::operator [] (size_type pos)
{
    auto ptr = array_elem_checked(_d.ptr, pos);
    return ptr ? json{rep_type{ptr, _d.ptr}} : json{};
}

template <>
json<jansson_backend>::const_reference
json<jansson_backend>::operator [] (size_type pos) const
{
    auto ptr = array_elem_checked(_d.ptr, pos);
    return ptr ? json{rep_type{ptr, _d.ptr}} : json{};
}

template <>
json<jansson_backend>::reference
json<jansson_backend>::operator [] (key_type const & key)
{
    auto ptr = object_elem_checked(_d.ptr, key.c_str(), key.size(), true);
    return ptr ? json{rep_type{ptr, _d.ptr}} : json{};
}

template <>
json<jansson_backend>::const_reference
json<jansson_backend>::operator [] (key_type const & key) const
{
    auto ptr = object_elem_checked(_d.ptr, key.c_str(), key.size(), false);
    return ptr ? json{rep_type{ptr, _d.ptr}} : json{};
}

////
template <>
json<jansson_backend>::reference
json<jansson_backend>::operator [] (char const * key)
{
    auto ptr = object_elem_checked(_d.ptr, key, std::strlen(key), true);
    return ptr ? json{rep_type{ptr, _d.ptr}} : json{};
}

template <>
json<jansson_backend>::const_reference
json<jansson_backend>::operator [] (char const * key) const
{
    auto ptr = object_elem_checked(_d.ptr, key, std::strlen(key), false);
    return ptr ? json{rep_type{ptr, _d.ptr}} : json{};
}

namespace details {

bool get (json_t * j, bool * result) noexcept
{
    if (! j)
        return false;

    if (!json_is_boolean(j))
        return false;

    *result = (json_is_true(j) != 0);
    return true;
}

bool get (json_t * j, std::intmax_t * result
    , std::intmax_t min, std::intmax_t max) noexcept
{
    if (! j)
        return false;

    if (!json_is_integer(j))
        return false;

    *result = static_cast<std::intmax_t>(json_integer_value(j));

    // JEYSON__THROW(error(errc::overflow));
    if (*result < min || *result > max)
        return false;

    return true;
}

bool get (json_t * j, double * result, double min, double max) noexcept
{
    if (! j)
        return false;

    if (!json_is_real(j))
        return false;

    *result = static_cast<double>(json_real_value(j));

    // JEYSON__THROW(error(errc::overflow));
    if (*result < min || *result > max)
        return false;

    return true;
}

} // details

template <>
pfs::optional<bool>
get<bool, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    bool result;
    return details::get(j._d.ptr, & result)
        ? pfs::optional<bool>{result} : pfs::nullopt;
}

template <>
pfs::optional<long long>
get<long long, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<long long>::min()
            , std::numeric_limits<long long>::max())
        ? pfs::optional<long long>{static_cast<long long>(result)} : pfs::nullopt;
}

template <>
pfs::optional<unsigned long long>
get<unsigned long long, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<unsigned long long>::min()
            , std::numeric_limits<unsigned long long>::max())
        ? pfs::optional<unsigned long long>{static_cast<unsigned long long>(result)}
        : pfs::nullopt;
}

template <>
pfs::optional<long>
get<long, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<long>::min()
            , std::numeric_limits<long>::max())
        ? pfs::optional<long>{static_cast<long>(result)} : pfs::nullopt;
}

template <>
pfs::optional<unsigned long>
get<unsigned long, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<unsigned long>::min()
            , std::numeric_limits<unsigned long>::max())
        ? pfs::optional<unsigned long>{static_cast<unsigned long>(result)}
        : pfs::nullopt;
}

template <>
pfs::optional<int>
get<int, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<int>::min()
            , std::numeric_limits<int>::max())
        ? pfs::optional<int>{static_cast<int>(result)} : pfs::nullopt;
}

template <>
pfs::optional<unsigned int>
get<unsigned int, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<unsigned int>::min()
            , std::numeric_limits<unsigned int>::max())
        ? pfs::optional<unsigned int>{static_cast<unsigned int>(result)}
        : pfs::nullopt;
}

template <>
pfs::optional<short>
get<short, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<short>::min()
            , std::numeric_limits<short>::max())
        ? pfs::optional<short>{static_cast<short>(result)} : pfs::nullopt;
}

template <>
pfs::optional<unsigned short>
get<unsigned short, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<unsigned short>::min()
            , std::numeric_limits<unsigned short>::max())
        ? pfs::optional<unsigned short>{static_cast<unsigned short>(result)}
        : pfs::nullopt;
}

template <>
pfs::optional<char>
get<char, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<char>::min()
            , std::numeric_limits<char>::max())
        ? pfs::optional<char>{static_cast<char>(result)} : pfs::nullopt;
}

template <>
pfs::optional<signed char>
get<signed char, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<signed char>::min()
            , std::numeric_limits<signed char>::max())
        ? pfs::optional<signed char>{static_cast<signed char>(result)} : pfs::nullopt;
}

template <>
pfs::optional<unsigned char>
get<unsigned char, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    std::intmax_t result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<unsigned char>::min()
            , std::numeric_limits<unsigned char>::max())
        ? pfs::optional<unsigned char>{static_cast<unsigned char>(result)}
        : pfs::nullopt;
}

template <>
pfs::optional<double>
get<double, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    double result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<double>::min()
            , std::numeric_limits<double>::max())
        ? pfs::optional<double>{result} : pfs::nullopt;
}

template <>
pfs::optional<float>
get<float, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    double result;
    return details::get(j._d.ptr, & result
            , std::numeric_limits<float>::min()
            , std::numeric_limits<float>::max())
        ? pfs::optional<float>{static_cast<float>(result)} : pfs::nullopt;
}

template <>
pfs::optional<std::string>
get<std::string, jansson_backend> (json<jansson_backend> const & j) noexcept
{
    if (!j._d.ptr)
        return pfs::nullopt;

    if (!json_is_string(j._d.ptr))
        return pfs::nullopt;

    return std::string(json_string_value(j._d.ptr), json_string_length(j._d.ptr));
}

////////////////////////////////////////////////////////////////////////////////
// Type quieries
////////////////////////////////////////////////////////////////////////////////

template <>
bool is_null (json<jansson_backend> const & j) noexcept
{
    return j._d.ptr ? json_is_null(j._d.ptr) : false;
}

template <>
bool is_bool (json<jansson_backend> const & j) noexcept
{
    return j._d.ptr ? json_is_boolean(j._d.ptr) : false;
}

template <>
bool is_integer (json<jansson_backend> const & j) noexcept
{
    return j._d.ptr ? json_is_integer(j._d.ptr) : false;
}

template <>
bool is_real (json<jansson_backend> const & j) noexcept
{
    return j._d.ptr ? json_is_real(j._d.ptr) : false;
}

template <>
bool is_string (json<jansson_backend> const & j) noexcept
{
    return j._d.ptr ? json_is_string(j._d.ptr) : false;
}

template <>
bool is_array (json<jansson_backend> const & j) noexcept
{
    return j._d.ptr ? json_is_array(j._d.ptr) : false;
}

template <>
bool is_object (json<jansson_backend> const & j) noexcept
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

    json j{rep_type{json_deep_copy(value._d.ptr), nullptr}};

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

////////////////////////////////////////////////////////////////////////////////
// Stringification
////////////////////////////////////////////////////////////////////////////////
static int json_dump_callback (char const * buffer, size_t size, void * data)
{
    auto output = static_cast<std::string *>(data);
    *output += std::string(buffer, size);
    return 0;
}

template <>
std::string to_string (json<jansson_backend> const & j) noexcept
{
    std::string result;
    json_dump_callback(j._d.ptr, json_dump_callback, & result, JSON_COMPACT);
    return result;
}

////////////////////////////////////////////////////////////////////////////////
// Parsing
////////////////////////////////////////////////////////////////////////////////

template <>
json<jansson_backend>
json<jansson_backend>::parse (std::string const & source, error * err) noexcept
{
    json_error_t jerror;

    auto j = json_loads(source.c_str(), 0, & jerror);

    if (!j) {
        if (err) {
            *err = error{errc::backend_error
                , fmt::format("parse error at line {}", jerror.line)
                , jerror.text};
        }

        return json<jansson_backend>{};
    }

    return json<jansson_backend>{json<jansson_backend>::rep_type{j, nullptr}};
}

template <>
json<jansson_backend>
json<jansson_backend>::parse (pfs::filesystem::path const & path, error * err) noexcept
{
    json_error_t jerror;

    auto j = json_load_file(pfs::filesystem::utf8_encode(path).c_str()
        ,     JSON_DECODE_ANY
            | JSON_REJECT_DUPLICATES
            | JSON_ALLOW_NUL
        , & jerror);

    if (!j) {
        if (err) {
            *err = error{errc::backend_error
                , fmt::format("parse error at line {} in file {}"
                    , jerror.line
                    , pfs::filesystem::utf8_encode(path))
                , jerror.text};
        }

        return json<jansson_backend>{};
    }

    return json<jansson_backend>{json<jansson_backend>::rep_type{j, nullptr}};
}

} // namespace jeyson
