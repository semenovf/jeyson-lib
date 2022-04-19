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
#include <algorithm>
#include <limits>
#include <sstream>
#include <cassert>
#include <cctype>
#include <cstdlib>

namespace jeyson {

#define BACKEND backend::jansson

#define NATIVE(x) ((x)._ptr)
#define INATIVE(x) (reinterpret_cast<BACKEND::rep *>(& x)->_ptr)
#define CINATIVE(x) (reinterpret_cast<BACKEND::rep const *>(& x)->_ptr)

static_assert(std::numeric_limits<std::intmax_t>::max()
    == std::numeric_limits<json_int_t>::max()
    , "");

inline bool case_eq (pfs::string_view const & a, pfs::string_view const & b)
{
    auto first1 = a.begin();
    auto last1 = a.end();
    auto first2 = b.begin();
    auto last2 = b.end();

    for (; first1 != last1 && first2 != last2; ++first1, ++first2) {
        if (std::tolower(*first1) != std::tolower(*first2))
            return false;
    }

    return first1 == last1 && first2 == last2;
}

namespace backend {

////////////////////////////////////////////////////////////////////////////////
// rep
////////////////////////////////////////////////////////////////////////////////
jansson::rep::rep () = default;

jansson::rep::rep (rep const & other)
{
    if (other._ptr)
        _ptr = json_deep_copy(other._ptr);
}

jansson::rep::rep (rep && other)
{
    if (other._ptr) {
        _ptr = other._ptr;
        other._ptr = nullptr;
    }
}

jansson::rep::rep (json_t * p)
    : _ptr(p)
{}

jansson::rep::~rep ()
{
    if (_ptr)
        json_decref(_ptr);

    _ptr = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// ref
////////////////////////////////////////////////////////////////////////////////
jansson::ref::ref () = default;

jansson::ref::ref (ref && other)
{
    _ptr = other._ptr;
    _parent = other._parent;

    if (_parent) {
        if (json_is_object(_parent))
            new (& index.key) key_type(std::move(other.index.key));
        else
            index.i = other.index.i;
    }

    other._ptr = nullptr;
    other._parent = nullptr;
}

jansson::ref::ref (json_t * ptr, json_t * parent, size_type index)
{
    if (ptr)
        _ptr = json_incref(ptr);

    if (parent) {
        _parent = json_incref(parent);
        ref::index.i = index;
    }
}

jansson::ref::ref (json_t * ptr, json_t * parent, std::string const & key)
{
    if (ptr)
        _ptr = json_incref(ptr);

    if (parent) {
        _parent = json_incref(parent);
        new (& index.key) key_type(key);
    }
}

jansson::ref::~ref ()
{
    if (_parent) {
        if (json_is_object(_parent))
            index.key.~basic_string();
    }

    if (_ptr) {
        json_decref(_ptr);
        _ptr = nullptr;
    }

    if (_parent) {
        json_decref(_parent);
        _parent = nullptr;
    }
}

} // namespace backend

////////////////////////////////////////////////////////////////////////////////
// JSON value
////////////////////////////////////////////////////////////////////////////////
template <>
json<BACKEND>::operator bool () const noexcept
{
    return NATIVE(*this) != nullptr;
}

//------------------------------------------------------------------------------
// Constructors, destructors, assignment operators
//------------------------------------------------------------------------------
template <>
json<BACKEND>::json () = default;

template <>
json<BACKEND>::json (std::nullptr_t) : rep_type(json_null())
{}

template <>
json<BACKEND>::json (bool value) : rep_type(json_boolean(value))
{}

template <>
json<BACKEND>::json (std::intmax_t value) : rep_type(json_integer(value))
{}

template <>
json<BACKEND>::json (double value) : rep_type(json_real(value))
{}

template <>
json<BACKEND>::json (std::string const & value)
    : rep_type(json_stringn_nocheck(value.c_str(), value.size()))
{}

template <>
json<BACKEND>::json (char const * value)
    : rep_type(json_string_nocheck(value))
{}

template <>
json<BACKEND>::json (char const * value, std::size_t n)
    : rep_type(json_stringn_nocheck(value, n))
{}

template <>
json<BACKEND>::json (json const & other) = default;

template <>
json<BACKEND>::json (json && other) = default;

template <>
json<BACKEND>::~json () = default;

template <>
json<BACKEND> & json<BACKEND>::operator = (json const & other)
{
    if (this != & other) {
        if (NATIVE(*this) != NATIVE(other)) {
            this->~json();

            if (NATIVE(other))
                NATIVE(*this) = json_deep_copy(NATIVE(other));
        }
    }

    return *this;
}

template <>
json<BACKEND> &
json<BACKEND>::operator = (json && other)
{
    if (this != & other) {
        if (NATIVE(*this) != NATIVE(other)) {
            this->~json();

            if (NATIVE(other)) {
                NATIVE(*this) = NATIVE(other);
                NATIVE(other) = nullptr;
            }
        }
    }

    return *this;
}

//------------------------------------------------------------------------------
// Modifiers
//------------------------------------------------------------------------------
template <>
void
json<BACKEND>::swap (json & other)
{
    json_t * tmp = NATIVE(*this);
    NATIVE(*this) = NATIVE(other);
    NATIVE(other) = tmp;
}

//------------------------------------------------------------------------------
// Save
//------------------------------------------------------------------------------
template <>
void json<BACKEND>::save (pfs::filesystem::path const & path
    , bool compact
    , int indent
    , int precision)
{
    std::size_t flags = JSON_ENCODE_ANY;

    if (compact) {
        flags |= JSON_COMPACT;
    } else {
        if (indent > 31)
            indent = 31;

        if (indent > 0)
            flags |= JSON_INDENT(indent);
    }

    if (precision > 31)
        precision = 31;

    if (precision > 0)
        flags |= JSON_REAL_PRECISION(precision);

    auto rc = json_dump_file(NATIVE(*this)
        , pfs::filesystem::utf8_encode(path).c_str()
        , flags);

    if (rc < 0) {
        error err {
              errc::backend_error
            , fmt::format("save JSON representation to file `{}` failure"
                , pfs::filesystem::utf8_encode(path))
        };
        JEYSON__THROW(err);
    }
}

//------------------------------------------------------------------------------
// Parsing
//------------------------------------------------------------------------------
template <>
json<BACKEND>
json<BACKEND>::parse (std::string const & source)
{
    json_error_t jerror;

    auto j = json_loads(source.c_str(), JSON_DECODE_ANY, & jerror);

    if (!j) {
        error err {
              errc::backend_error
            , fmt::format("parse error at line {}", jerror.line)
            , jerror.text
        };

        JEYSON__THROW(err);
    }

    json<BACKEND> result;
    NATIVE(result) = j;

    return result;
}

template <>
json<BACKEND>
json<BACKEND>::parse (pfs::filesystem::path const & path)
{
    json_error_t jerror;

    auto j = json_load_file(pfs::filesystem::utf8_encode(path).c_str()
        ,     JSON_DECODE_ANY
            | JSON_REJECT_DUPLICATES
            | JSON_ALLOW_NUL
        , & jerror);

    if (!j) {
        error err{
              errc::backend_error
            , fmt::format("parse error at line {} in file `{}`"
                , jerror.line
                , pfs::filesystem::utf8_encode(path))
            , jerror.text};

        JEYSON__THROW(err);
    }

    json<BACKEND> result;
    NATIVE(result) = j;

    return result;
}

//------------------------------------------------------------------------------
// Comparison operators
//------------------------------------------------------------------------------
template <>
bool
operator == (json<BACKEND> const & lhs, json<BACKEND> const & rhs)
{
    if (!NATIVE(lhs) && !NATIVE(rhs))
        return true;

    return json_equal(NATIVE(lhs), NATIVE(rhs)) == 1;
}

////////////////////////////////////////////////////////////////////////////////
// JSON reference
////////////////////////////////////////////////////////////////////////////////
template <>
json_ref<BACKEND>::json_ref () = default;

template <>
json_ref<BACKEND>::json_ref (json_ref &&) = default;

template <>
json_ref<BACKEND>::json_ref (BACKEND::ref && other)
    : BACKEND::ref(std::move(other))
{}

template <>
json_ref<BACKEND>::~json_ref () = default;

template <>
json_ref<BACKEND>::operator bool () const noexcept
{
    return _ptr != nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Traits interface
////////////////////////////////////////////////////////////////////////////////
template <>
bool
traits_interface<BACKEND>::is_null () const noexcept
{
    return CINATIVE(*this) ? json_is_null(CINATIVE(*this)) : false;
}

template <>
bool
traits_interface<BACKEND>::is_bool () const noexcept
{
    return CINATIVE(*this) ? json_is_boolean(CINATIVE(*this)) : false;
}

template <>
bool
traits_interface<BACKEND>::is_integer () const noexcept
{
    return CINATIVE(*this) ? json_is_integer(CINATIVE(*this)) : false;
}

template <>
bool
traits_interface<BACKEND>::is_real () const noexcept
{
    return CINATIVE(*this) ? json_is_real(CINATIVE(*this)) : false;
}

template <>
bool
traits_interface<BACKEND>::is_string () const noexcept
{
    return CINATIVE(*this) ? json_is_string(CINATIVE(*this)) : false;
}

template <>
bool
traits_interface<BACKEND>::is_array () const noexcept
{
    return CINATIVE(*this) ? json_is_array(CINATIVE(*this)) : false;
}

template <>
bool
traits_interface<BACKEND>::is_object () const noexcept
{
    return CINATIVE(*this) ? json_is_object(CINATIVE(*this)) : false;
}

////////////////////////////////////////////////////////////////////////////////
// Modifiers interface
////////////////////////////////////////////////////////////////////////////////
template <>
void
modifiers_interface<BACKEND>::insert (key_type const & key, json<BACKEND> const & value)
{
    if (!value) {
        error err {errc::invalid_argument, "attempt to insert unitialized value"};
        JEYSON__THROW(err);
    }

    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    if (!json_is_object(INATIVE(*this))) {
        error err{errc::incopatible_type, "object expected"};
        JEYSON__THROW(err);
    }

    auto copy = json_deep_copy(NATIVE(value));

    if (!copy) {
        error err{errc::backend_error, "deep copy failure"};
        JEYSON__THROW(err);
    }

    auto rc = json_object_setn_new_nocheck(INATIVE(*this)
        , key.c_str()
        , key.size()
        , copy);

    if (rc != 0) {
        error err{errc::backend_error, "object insertion failure"};
        JEYSON__THROW(err);
    }
}

template <>
void
modifiers_interface<BACKEND>::insert (key_type const & key, json<BACKEND> && value)
{
    if (!value) {
        error err {errc::invalid_argument, "attempt to insert unitialized value"};
        JEYSON__THROW(err);
    }

    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    if (!json_is_object(INATIVE(*this))) {
        error err{errc::incopatible_type, "object expected"};
        JEYSON__THROW(err);
    }

    auto rc = json_object_setn_new_nocheck(INATIVE(*this)
        , key.c_str()
        , key.size()
        , NATIVE(value));

    if (rc != 0) {
        error err{errc::backend_error, "object insertion failure"};
        JEYSON__THROW(err);
    }

    NATIVE(value) = nullptr;
}

template <>
void
modifiers_interface<BACKEND>::push_back (json<BACKEND> const & value)
{
    if (!value) {
        error err {errc::invalid_argument, "attempt to add unitialized value"};
        JEYSON__THROW(err);
    }

    if (! INATIVE(*this))
        INATIVE(*this) = json_array();

    if (!json_is_array(INATIVE(*this))) {
        error err{errc::incopatible_type, "array expected"};
        JEYSON__THROW(err);
    }

    auto copy = json_deep_copy(NATIVE(value));

    if (!copy) {
        error err{errc::backend_error, "deep copy failure"};
        JEYSON__THROW(err);
    }

    auto rc = json_array_append_new(INATIVE(*this), copy);

    if (rc != 0) {
        error err{errc::backend_error, "array append failure"};
        JEYSON__THROW(err);
    }
}

template <>
void
modifiers_interface<BACKEND>::push_back (json<BACKEND> && value)
{
    if (!value) {
        error err {errc::invalid_argument, "attempt to add unitialized value"};
        JEYSON__THROW(err);
    }

    if (! INATIVE(*this))
        INATIVE(*this) = json_array();

    if (!json_is_array(INATIVE(*this))) {
        error err{errc::incopatible_type, "array expected"};
        JEYSON__THROW(err);
    }

    auto rc = json_array_append_new(INATIVE(*this), NATIVE(value));

    if (rc != 0) {
        error err{errc::backend_error, "array append failure"};
        JEYSON__THROW(err);
    }

    NATIVE(value) = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
// Capacity interface
////////////////////////////////////////////////////////////////////////////////
template <>
capacity_interface<BACKEND>::size_type
capacity_interface<BACKEND>::size () const noexcept
{
    if (!CINATIVE(*this))
        return 0;

    if (json_is_object(CINATIVE(*this)))
        return json_object_size(CINATIVE(*this));

    if (json_is_array(CINATIVE(*this)))
        return json_array_size(CINATIVE(*this));

    // Scalar types
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
// Converter interface
////////////////////////////////////////////////////////////////////////////////
static int json_dump_callback (char const * buffer, size_t size, void * data)
{
    auto output = static_cast<std::string *>(data);
    *output += std::string(buffer, size);
    return 0;
}

template <>
std::string
converter_interface<BACKEND>::to_string () const
{
    std::string result;

    if (CINATIVE(*this)) {
        auto rc = json_dump_callback(CINATIVE(*this)
            , json_dump_callback, & result, JSON_COMPACT);

        if (rc != 0) {
            error err{errc::backend_error, "stringification failure"};
            JEYSON__THROW(err);
        }
    }

    return result;
};

////////////////////////////////////////////////////////////////////////////////
// Lexical cast (specializations)
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// nullptr_t
//------------------------------------------------------------------------------
template <>
std::nullptr_t
lexical_cast<std::nullptr_t>::operator () () const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
lexical_cast<std::nullptr_t>::operator () (std::nullptr_t, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
lexical_cast<std::nullptr_t>::operator () (bool, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
lexical_cast<std::nullptr_t>::operator () (std::intmax_t, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
lexical_cast<std::nullptr_t>::operator () (double, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
lexical_cast<std::nullptr_t>::operator () (pfs::string_view const &, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
lexical_cast<std::nullptr_t>::operator () (std::size_t, bool, bool *) const noexcept
{
    return nullptr;
}

//------------------------------------------------------------------------------
// bool
//------------------------------------------------------------------------------
template <>
bool
lexical_cast<bool>::operator () () const noexcept
{
    return false;
}

template <>
bool
lexical_cast<bool>::operator () (std::nullptr_t, bool *) const noexcept
{
    return false;
}

template <>
bool
lexical_cast<bool>::operator () (bool v, bool *) const noexcept
{
    return v;
}

template <>
bool
lexical_cast<bool>::operator () (std::intmax_t v, bool *) const noexcept
{
    return static_cast<bool>(v);
}

template <>
bool
lexical_cast<bool>::operator () (double v, bool *) const noexcept
{
    return static_cast<bool>(v);
}

template <>
bool
lexical_cast<bool>::operator () (pfs::string_view const & v, bool * success) const noexcept
{
    if (v.empty())
        return false;

    if (case_eq(v, "false") || case_eq(v, "no") || case_eq(v, "off"))
        return false;

    if (case_eq(v, "true") || case_eq(v, "yes") || case_eq(v, "on"))
        return true;

    *success = false;
    return this->operator() ();
}

template <>
bool
lexical_cast<bool>::operator () (std::size_t size, bool, bool *) const noexcept
{
    return size > 0;
}

//------------------------------------------------------------------------------
// std::intmax_t
//------------------------------------------------------------------------------
template <>
std::intmax_t
lexical_cast<std::intmax_t>::operator () () const noexcept
{
    return 0;
}

template <>
std::intmax_t
lexical_cast<std::intmax_t>::operator () (std::nullptr_t, bool *) const noexcept
{
    return 0;
}

template <>
std::intmax_t
lexical_cast<std::intmax_t>::operator () (bool v, bool *) const noexcept
{
    return v ? 1 : 0;
}

template <>
std::intmax_t
lexical_cast<std::intmax_t>::operator () (std::intmax_t v, bool *) const noexcept
{
    return v;
}

template <>
std::intmax_t
lexical_cast<std::intmax_t>::operator () (double v, bool * success) const noexcept
{
    if (v >= static_cast<double>(std::numeric_limits<std::intmax_t>::min())
            && v <= static_cast<double>(std::numeric_limits<std::intmax_t>::max())) {
        return static_cast<std::intmax_t>(v);
    }

    *success = false;
    return this->operator()();
}

template <>
std::intmax_t
lexical_cast<std::intmax_t>::operator () (pfs::string_view const & v, bool * success) const noexcept
{
    if (!v.empty()) {
        char * endptr = nullptr;
        errno = 0;

        if (sizeof(std::intmax_t) == sizeof(long int)) {
            auto n = std::strtol(v.data(), & endptr, 10);

            if (endptr == (v.data() + v.size()) && !errno)
                return static_cast<std::intmax_t>(n);
        } else {
            auto n = std::strtoll(v.data(), & endptr, 10);

            if (endptr == (v.data() + v.size()) && !errno)
                return static_cast<std::intmax_t>(n);
        }
    }

    *success = false;
    return this->operator()();
}

template <>
std::intmax_t
lexical_cast<std::intmax_t>::operator () (std::size_t size, bool, bool *) const noexcept
{
    return static_cast<std::intmax_t>(size);
}

//------------------------------------------------------------------------------
// double
//------------------------------------------------------------------------------
template <>
double
lexical_cast<double>::operator () () const noexcept
{
    return 0.0;
}

template <>
double
lexical_cast<double>::operator () (std::nullptr_t, bool *) const noexcept
{
    return 0.0;
}

template <>
double
lexical_cast<double>::operator () (bool v, bool *) const noexcept
{
    return v ? 1.0 : 0.0;
}

template <>
double
lexical_cast<double>::operator () (std::intmax_t v, bool *) const noexcept
{
    return static_cast<double>(v);
}

template <>
double
lexical_cast<double>::operator () (double v, bool *) const noexcept
{
    return v;
}

template <>
double
lexical_cast<double>::operator () (pfs::string_view const & v, bool * success) const noexcept
{
    if (!v.empty()) {
        errno = 0;
        char * endptr = nullptr;
        auto n = std::strtod(v.data(), & endptr);

        auto b = endptr == (v.data() + v.size()) && !errno;

        if (endptr == (v.data() + v.size()) && !errno)
            return n;
    }

    *success = false;
    return this->operator()();
}

template <>
double
lexical_cast<double>::operator () (std::size_t size, bool, bool *) const noexcept
{
    return static_cast<std::intmax_t>(size);
}

//------------------------------------------------------------------------------
// std::string
//------------------------------------------------------------------------------
template <>
std::string
lexical_cast<std::string>::operator () () const noexcept
{
    return "";
}

template <>
std::string
lexical_cast<std::string>::operator () (std::nullptr_t, bool *) const noexcept
{
    return "";
}

template <>
std::string
lexical_cast<std::string>::operator () (bool v, bool *) const noexcept
{
    return v ? "true" : "false";
}

template <>
std::string
lexical_cast<std::string>::operator () (std::intmax_t v, bool *) const noexcept
{
    return std::to_string(v);
}

template <>
std::string
lexical_cast<std::string>::operator () (double v, bool *) const noexcept
{
    return std::to_string(v);
}

template <>
std::string
lexical_cast<std::string>::operator () (pfs::string_view const & v, bool *) const noexcept
{
    return std::string(v.data(), v.size());
}

template <>
std::string
lexical_cast<std::string>::operator () (std::size_t size, bool, bool *) const noexcept
{
    return std::to_string(size);
}

////////////////////////////////////////////////////////////////////////////////
// Element accessor interface
////////////////////////////////////////////////////////////////////////////////
template <>
bool
element_accessor_interface<BACKEND>::bool_value () const noexcept
{
    JEYSON__ASSERT(json_is_boolean(CINATIVE(*this)), "Boolean value expected");
    return json_is_true(CINATIVE(*this)) ? true : false;
}

template <>
std::intmax_t
element_accessor_interface<BACKEND>::integer_value () const noexcept
{
    JEYSON__ASSERT(json_is_integer(CINATIVE(*this)), "Integer value expected");
    return static_cast<std::intmax_t>(json_integer_value(CINATIVE(*this)));
}

template <>
double
element_accessor_interface<BACKEND>::real_value () const noexcept
{
    JEYSON__ASSERT(json_is_real(CINATIVE(*this)), "Real value expected");
    return json_real_value(CINATIVE(*this));
}

template <>
pfs::string_view
element_accessor_interface<BACKEND>::string_value () const noexcept
{
    JEYSON__ASSERT(json_is_string(CINATIVE(*this)), "String value expected");
    return pfs::string_view(json_string_value(CINATIVE(*this))
        , json_string_length(CINATIVE(*this)));
}

template <>
std::size_t
element_accessor_interface<BACKEND>::array_size () const noexcept
{
    JEYSON__ASSERT(json_is_array(CINATIVE(*this)), "Array expected");
    return json_array_size(CINATIVE(*this));
}

template <>
std::size_t
element_accessor_interface<BACKEND>::object_size () const noexcept
{
    JEYSON__ASSERT(json_is_object(CINATIVE(*this)), "Object expected");
    return json_object_size(CINATIVE(*this));
}

template <>
element_accessor_interface<BACKEND>::reference
element_accessor_interface<BACKEND>::operator [] (size_type pos) noexcept
{
    if (!INATIVE(*this) || !json_is_array(INATIVE(*this)))
        return reference{};

    if (pos >= json_array_size(INATIVE(*this))) {
        // Fill with null values
        while (json_array_size(INATIVE(*this)) <= pos) {
            auto rc = json_array_append_new(INATIVE(*this), json_null());
            JEYSON__ASSERT(rc == 0, "json_array_append_new() failure");
        }
    }

    auto ptr = json_array_get(INATIVE(*this), pos);

    if (!ptr)
        return reference{};

    return reference{BACKEND::ref{ptr, INATIVE(*this), pos}};
}

template <>
element_accessor_interface<BACKEND>::const_reference
element_accessor_interface<BACKEND>::operator [] (size_type pos) const noexcept
{
    if (!CINATIVE(*this) || !json_is_array(CINATIVE(*this)))
        return reference{};

    auto ptr = json_array_get(CINATIVE(*this), pos);

    if (!ptr)
        return reference{};

    return reference{BACKEND::ref{ptr, CINATIVE(*this), pos}};
}

template <>
element_accessor_interface<BACKEND>::reference
element_accessor_interface<BACKEND>::operator [] (pfs::string_view key) noexcept
{
    if (!INATIVE(*this) || !json_is_object(INATIVE(*this)))
        return reference{};

    auto ptr = json_object_getn(INATIVE(*this), key.data(), key.length());

    // Not found, insert new `null` element
    if (!ptr) {
        auto rc = json_object_setn_new_nocheck(INATIVE(*this)
            , key.data(), key.length(), json_null());

        if (rc != 0)
            return reference{};

        auto ptr = json_object_getn(INATIVE(*this), key.data(), key.length());

        JEYSON__ASSERT(ptr, "");
    }

    return reference{BACKEND::ref{ptr, INATIVE(*this), key_type(key.data(), key.length())}};
}

template <>
element_accessor_interface<BACKEND>::reference
element_accessor_interface<BACKEND>::operator [] (key_type const & key) noexcept
{
    return this->operator[] (pfs::string_view{key});
}

template <>
element_accessor_interface<BACKEND>::reference
element_accessor_interface<BACKEND>::operator [] (char const * key) noexcept
{
    return this->operator[] (pfs::string_view{key});
}

template <>
element_accessor_interface<BACKEND>::const_reference
element_accessor_interface<BACKEND>::operator [] (pfs::string_view key) const noexcept
{
    if (!CINATIVE(*this) || !json_is_object(CINATIVE(*this)))
        return reference{};

    auto ptr = json_object_getn(CINATIVE(*this), key.data(), key.length());

    // Not found
    if (!ptr)
        return reference{};

    return reference{BACKEND::ref{ptr, CINATIVE(*this), key_type(key.data(), key.length())}};
}

template <>
element_accessor_interface<BACKEND>::const_reference
element_accessor_interface<BACKEND>::operator [] (key_type const & key) const noexcept
{
    if (!CINATIVE(*this) || !json_is_object(CINATIVE(*this)))
        return reference{};

    auto ptr = json_object_getn(CINATIVE(*this), key.c_str(), key.size());

    // Not found
    if (!ptr)
        return reference{};

    return reference{BACKEND::ref{ptr, CINATIVE(*this), key}};
}

template <>
element_accessor_interface<BACKEND>::const_reference
element_accessor_interface<BACKEND>::operator [] (char const * key) const noexcept
{
    if (!CINATIVE(*this) || !json_is_object(CINATIVE(*this)))
        return reference{};

    auto ptr = json_object_getn(CINATIVE(*this), key, std::strlen(key));

    // Not found
    if (!ptr)
        return reference{};

    return reference{BACKEND::ref{ptr, CINATIVE(*this), key_type(key, std::strlen(key))}};
}

template <>
element_accessor_interface<BACKEND>::reference
element_accessor_interface<BACKEND>::at (size_type pos) const
{
    if (!CINATIVE(*this) || !json_is_array(CINATIVE(*this))) {
        error err{errc::incopatible_type, "array expected"};
        JEYSON__THROW(err);
    }

    auto ptr = json_array_get(CINATIVE(*this), pos);

    if (!ptr) {
        error err{errc::out_of_range};
        JEYSON__THROW(err);
    }

    return reference{BACKEND::ref{ptr, CINATIVE(*this), pos}};
}

template <>
element_accessor_interface<BACKEND>::reference
element_accessor_interface<BACKEND>::at (pfs::string_view key) const
{
    if (!CINATIVE(*this) || !json_is_object(CINATIVE(*this))) {
        error err{errc::incopatible_type, "object expected"};
        JEYSON__THROW(err);
    }

    auto ptr = json_object_getn(CINATIVE(*this), key.data(), key.length());

    if (!ptr) {
        error err{errc::out_of_range};
        JEYSON__THROW(err);
    }

    return reference{BACKEND::ref{ptr, CINATIVE(*this), key_type(key.data(), key.length())}};
}

template <>
element_accessor_interface<BACKEND>::reference
element_accessor_interface<BACKEND>::at (key_type const & key) const
{
    return at(pfs::string_view{key});
}

template <>
element_accessor_interface<BACKEND>::reference
element_accessor_interface<BACKEND>::at (char const * key) const
{
    return at(pfs::string_view{key});
}

// template <>
// json<BACKEND> &
// json<BACKEND>::operator = (std::nullptr_t)
// {
//     json j{nullptr};
//     this->swap(j);
//     return *this;
// }

// template <>
// json<BACKEND> & json<BACKEND>::operator = (json const & other)
// {
//     if (this != & other) {
//         if (_d.ptr != other._d.ptr) {
//             // Reference
//             if (_d.refdata) {
//                 JEYSON__ASSERT(_d.refdata->parent, "");
//                 JEYSON__ASSERT(json_is_array(_d.refdata->parent)
//                     || json_is_object(_d.refdata->parent), "");
//
//                 if (json_is_array(_d.refdata->parent)) {
//                     auto rc = json_array_remove(_d.refdata->parent
//                         , _d.refdata->index.i);
//
//                     if (rc != 0)
//                         JEYSON__THROW((error{errc::backend_error, "remove array element failure"}));
//
//                     rc = json_array_set(_d.refdata->parent
//                         , _d.refdata->index.i
//                         , json_deep_copy(other._d.ptr));
//
//                     if (rc != 0)
//                         JEYSON__THROW((error{errc::backend_error, "update array element failure"}));
//                 } else {
//                     auto rc = json_object_deln(_d.refdata->parent
//                         , _d.refdata->index.key.c_str()
//                         , _d.refdata->index.key.size());
//
//                     if (rc != 0)
//                         JEYSON__THROW((error{errc::backend_error, "remove object element failure"}));
//
//                     rc = json_object_setn(_d.refdata->parent
//                         , _d.refdata->index.key.c_str()
//                         , _d.refdata->index.key.size()
//                         , json_deep_copy(other._d.ptr));
//
//                     if (rc != 0)
//                         JEYSON__THROW((error{errc::backend_error, "update object element failure"}));
//                 }
//             } else {
//                 this->~json();
//                 _d.ptr = json_deep_copy(other._d.ptr);
//                 _d.refdata = other._d.refdata;
//             }
//         }
//     }
//
//     return *this;
// }
//
// template <>
// json<BACKEND> & json<BACKEND>::operator = (json && other)
// {
//     if (this != & other) {
//         if (_d.ptr != other._d.ptr) {
//             // Reference
//             if (_d.refdata) {
//                 JEYSON__ASSERT(_d.refdata->parent, "");
//                 JEYSON__ASSERT(json_is_array(_d.refdata->parent)
//                     || json_is_object(_d.refdata->parent), "");
//
//                 if (json_is_array(_d.refdata->parent)) {
//                     auto rc = json_array_remove(_d.refdata->parent
//                         , _d.refdata->index.i);
//
//                     if (rc != 0)
//                         JEYSON__THROW((error{errc::backend_error, "remove array element failure"}));
//
//                     rc = json_array_set(_d.refdata->parent
//                         , _d.refdata->index.i
//                         , other._d.ptr);
//
//                     if (rc != 0)
//                         JEYSON__THROW((error{errc::backend_error, "update array element failure"}));
//                 } else {
//                     auto rc = json_object_deln(_d.refdata->parent
//                         , _d.refdata->index.key.c_str()
//                         , _d.refdata->index.key.size());
//
//                     if (rc != 0)
//                         JEYSON__THROW((error{errc::backend_error, "remove object element failure"}));
//
//                     rc = json_object_setn(_d.refdata->parent
//                         , _d.refdata->index.key.c_str()
//                         , _d.refdata->index.key.size()
//                         , other._d.ptr);
//
//                     if (rc != 0)
//                         JEYSON__THROW((error{errc::backend_error, "update object element failure"}));
//                 }
//
//                 other._d.ptr = nullptr;
//             } else {
//                 this->~json();
//
//                 if (other._d.ptr) {
//                     _d.ptr = other._d.ptr;
//                     _d.refdata = std::move(other._d.refdata);
//                     other._d.ptr = nullptr;
//                 }
//             }
//         }
//     }
//
//     return *this;
// }
} // namespace jeyson
