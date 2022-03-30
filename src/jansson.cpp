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

jansson_backend::rep_type::rep_type () = default;

jansson_backend::rep_type::rep_type (rep_type const & other)
    : ptr(other.ptr)
    , refdata(other.refdata)
{}

jansson_backend::rep_type::rep_type (rep_type && other)
    : ptr(other.ptr)
    , refdata(std::move(other.refdata))
{
    other.ptr = nullptr;
}

jansson_backend::rep_type::rep_type (json_t * p)
    : ptr(p)
{}

jansson_backend::rep_type::rep_type (json_t * p, json_t * parent, size_type index)
    : ptr(p)
{
    refdata = std::make_shared<refdata_type>();
    refdata->parent = parent;
    refdata->index.i = index;
}

jansson_backend::rep_type::rep_type (json_t * p, json_t * parent, std::string const & key)
    : ptr(p)
{
    refdata = std::make_shared<refdata_type>();
    refdata->parent = parent;
    refdata->index.key = key;
}

////////////////////////////////////////////////////////////////////////////////
// Constructors, destructors, assignment operators
////////////////////////////////////////////////////////////////////////////////
template <>
std::function<void(error)> json<jansson_backend>::failure = [] (error) {};

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
    : _d(json_null())
{}

template <>
json<jansson_backend>::json (bool value)
    : _d(json_boolean(value))
{}

template <>
json<jansson_backend>::json (std::intmax_t value)
    : _d(json_integer(value))
{}

template <>
json<jansson_backend>::json (double value)
    : _d(json_real(value))
{}

template <>
json<jansson_backend>::json (std::string const & value)
    : _d(json_stringn(value.c_str(), value.size()))
{}

template <>
json<jansson_backend>::json (char const * value)
    : _d(json_string(value))
{}

template <>
json<jansson_backend>::json (char const * value, std::size_t n)
    : _d(json_stringn_nocheck(value, n))
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
        _d.refdata = std::move(other._d.refdata);
        other._d.ptr = nullptr;
    }
}

template <>
json<jansson_backend>::~json ()
{
    if (_d.ptr && !_d.refdata)
        json_decref(_d.ptr);
}

template <>
json<jansson_backend> & json<jansson_backend>::operator = (json const & other)
{
    if (this != & other) {
        if (_d.ptr != other._d.ptr) {
            // Reference
            if (_d.refdata) {
                assert(_d.refdata->parent);
                assert(json_is_array(_d.refdata->parent) || json_is_object(_d.refdata->parent));

                if (json_is_array(_d.refdata->parent)) {
                    auto rc = json_array_set(_d.refdata->parent
                        , _d.refdata->index.i
                        , json_deep_copy(other._d.ptr));

                    if (rc != 0) {
                        JEYSON__THROW((error{errc::backend_error, "update array element failure"}));
                    }
                } else {
                    auto rc = json_object_setn(_d.refdata->parent
                        , _d.refdata->index.key.c_str()
                        , _d.refdata->index.key.size()
                        , json_deep_copy(other._d.ptr));

                    if (rc != 0) {
                        JEYSON__THROW((error{errc::backend_error, "update object element failure"}));
                    }
                }
            } else {
                this->~json();
                _d.ptr = json_deep_copy(other._d.ptr);
                _d.refdata = other._d.refdata;
            }
        }
    }

    return *this;
}

template <>
json<jansson_backend> & json<jansson_backend>::operator = (json && other)
{
    if (this != & other) {
        if (_d.ptr != other._d.ptr) {
            // Reference
            if (_d.refdata) {
                assert(_d.refdata->parent);
                assert(json_is_array(_d.refdata->parent) || json_is_object(_d.refdata->parent));

                if (json_is_array(_d.refdata->parent)) {
                    auto rc = json_array_set(_d.refdata->parent
                        , _d.refdata->index.i
                        , other._d.ptr);

                    if (rc != 0) {
                        JEYSON__THROW((error{errc::backend_error, "update array element failure"}));
                    }
                } else {
                    auto rc = json_object_setn(_d.refdata->parent
                        , _d.refdata->index.key.c_str()
                        , _d.refdata->index.key.size()
                        , other._d.ptr);

                    if (rc != 0) {
                        JEYSON__THROW((error{errc::backend_error, "update object element failure"}));
                    }
                }

                other._d.ptr = nullptr;
            } else {
                this->~json();

                if (other._d.ptr) {
                    _d.ptr = other._d.ptr;
                    _d.refdata = std::move(other._d.refdata);
                    other._d.ptr = nullptr;
                }
            }
        }
    }

    return *this;
}

template <>
json<jansson_backend>::operator bool () const noexcept
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
    if (!_d.ptr) {
        _d.ptr = json_array();
    }

    auto ptr = array_elem_checked(_d.ptr, pos);
    return ptr ? json{rep_type{ptr, _d.ptr, pos}} : json{};
}

template <>
json<jansson_backend>::const_reference
json<jansson_backend>::operator [] (size_type pos) const
{
    auto ptr = array_elem_checked(_d.ptr, pos);
    return ptr ? json{rep_type{ptr, _d.ptr, pos}} : json{};
}

template <>
json<jansson_backend>::reference
json<jansson_backend>::operator [] (key_type const & key)
{
    if (!_d.ptr) {
        _d.ptr = json_object();
    }

    auto ptr = object_elem_checked(_d.ptr, key.c_str(), key.size(), true);
    return ptr ? json{rep_type{ptr, _d.ptr, key}} : json{};
}

template <>
json<jansson_backend>::const_reference
json<jansson_backend>::operator [] (key_type const & key) const
{
    auto ptr = object_elem_checked(_d.ptr, key.c_str(), key.size(), false);
    return ptr ? json{rep_type{ptr, _d.ptr, key}} : json{};
}

////
template <>
json<jansson_backend>::reference
json<jansson_backend>::operator [] (char const * key)
{
    if (!_d.ptr) {
        _d.ptr = json_object();
    }

    auto ptr = object_elem_checked(_d.ptr, key, std::strlen(key), true);
    return ptr ? json{rep_type{ptr, _d.ptr, std::string{key}}} : json{};
}

template <>
json<jansson_backend>::const_reference
json<jansson_backend>::operator [] (char const * key) const
{
    auto ptr = object_elem_checked(_d.ptr, key, std::strlen(key), false);
    return ptr ? json{rep_type{ptr, _d.ptr, std::string{key}}} : json{};
}

namespace details {

std::intmax_t get (json_t * j
    , std::intmax_t min, std::intmax_t max
    , bool * success)
{
    if (success)
        *success = true;

    if (! j) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::invalid_argument));

        return false;
    }

    if (!json_is_integer(j)) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::incopatible_type));

        return false;
    }

    auto result = static_cast<std::intmax_t>(json_integer_value(j));

    if (result < min || result > max) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::overflow));

        result = 0;
    }

    return result;
}

double get (json_t * j, double min, double max, bool * success)
{
    if (success)
        *success = true;

    if (! j) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::invalid_argument));

        return false;
    }

    if (!json_is_real(j)) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::incopatible_type));

        return false;
    }

    auto result = static_cast<double>(json_real_value(j));

    if (result < min || result > max) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::overflow));

        result = 0;
    }

    return result;
}

} // details

template <>
bool
get<bool, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    if (success)
        *success = true;

    if (! j._d.ptr) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::invalid_argument));

        return false;
    }

    if (!json_is_boolean(j._d.ptr)) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::incopatible_type));

        return false;
    }

    return static_cast<bool>(json_is_true(j._d.ptr));
}

template <>
long long
get<long long, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<long long>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<long long>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<long long>::max())
        , success));
}

template <>
unsigned long long
get<unsigned long long, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<unsigned long long>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<long long>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<long long>::max())
        , success));
}

template <>
long
get<long, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<long>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<long>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<long>::max())
        , success));
}

template <>
unsigned long
get<unsigned long, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<unsigned long>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<long>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<long>::max())
        , success));
}

template <>
int
get<int, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<int>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<int>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<int>::max())
        , success));
}

template <>
unsigned int
get<unsigned int, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<unsigned int>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<int>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<int>::max())
        , success));
}

template <>
short
get<short, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<short>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<short>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<short>::max())
        , success));
}

template <>
unsigned short
get<unsigned short, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<unsigned short>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<short>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<short>::max())
        , success));
}

template <>
char
get<char, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<char>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<char>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<char>::max())
        , success));
}

template <>
signed char
get<signed char, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<signed char>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<signed char>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<signed char>::max())
        , success));
}

template <>
unsigned char
get<unsigned char, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<unsigned char>(details::get(j._d.ptr
        , static_cast<std::intmax_t>(std::numeric_limits<signed char>::min())
        , static_cast<std::intmax_t>(std::numeric_limits<signed char>::max())
        , success));
}

template <>
double
get<double, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return details::get(j._d.ptr
        , std::numeric_limits<double>::min()
        , std::numeric_limits<double>::max()
        , success);
}

template <>
float
get<float, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    return static_cast<float>(details::get(j._d.ptr
        , static_cast<double>(std::numeric_limits<float>::min())
        , static_cast<double>(std::numeric_limits<float>::max())
        , success));
}

template <>
std::string
get<std::string, jansson_backend> (json<jansson_backend> const & j, bool * success)
{
    if (success)
        *success = true;

    if (!j._d.ptr) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::invalid_argument));

        return std::string{};
    }

    if (!json_is_string(j._d.ptr)) {
        if (success)
            *success = false;
        else
            JEYSON__THROW(error(errc::incopatible_type));

        return std::string{};
    }

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
    if (!value) {
        error err {errc::invalid_argument};
        JEYSON__THROW(err);
        return;
    }

    if (!_d.ptr)
        _d.ptr = json_array();

    if (!is_array(*this)) {
        error err {errc::incopatible_type};
        JEYSON__THROW(err);
        return;
    }

    auto rc = json_array_append_new(_d.ptr, value._d.ptr);

    assert(rc == 0);

    value._d.ptr = nullptr;
}

template <>
void
json<jansson_backend>::push_back (json const & value)
{
    if (!value) {
        error err {errc::invalid_argument};
        JEYSON__THROW(err);
        return;
    }

    if (!_d.ptr)
        _d.ptr = json_array();

    if (!is_array(*this)) {
        error err {errc::incopatible_type};
        JEYSON__THROW(err);
        return;
    }

    json j{rep_type{json_deep_copy(value._d.ptr)}};

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

    if (j) {
        json_dump_callback(j._d.ptr, json_dump_callback, & result, JSON_COMPACT);
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////
// Parsing
////////////////////////////////////////////////////////////////////////////////

template <>
json<jansson_backend>
json<jansson_backend>::parse (std::string const & source)
{
    json_error_t jerror;

    auto j = json_loads(source.c_str(), 0, & jerror);

    if (!j) {
        error err{errc::backend_error
            , fmt::format("parse error at line {}", jerror.line)
            , jerror.text};
        failure(err);
        return json<jansson_backend>{};
    }

    return json<jansson_backend>{json<jansson_backend>::rep_type{j}};
}

template <>
json<jansson_backend>
json<jansson_backend>::parse (pfs::filesystem::path const & path)
{
    json_error_t jerror;

    auto j = json_load_file(pfs::filesystem::utf8_encode(path).c_str()
        ,     JSON_DECODE_ANY
            | JSON_REJECT_DUPLICATES
            | JSON_ALLOW_NUL
        , & jerror);

    if (!j) {
        error err{errc::backend_error
            , fmt::format("parse error at line {} in file {}"
                , jerror.line
                , pfs::filesystem::utf8_encode(path))
            , jerror.text};
        failure(err);
        return json<jansson_backend>{};
    }

    return json<jansson_backend>{json<jansson_backend>::rep_type{j}};
}

} // namespace jeyson
