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
#define INATIVE(x) (reinterpret_cast<BACKEND::basic_rep *>(& x)->_ptr)
#define CINATIVE(x) (reinterpret_cast<BACKEND::basic_rep const *>(& x)->_ptr)

static_assert(std::numeric_limits<std::intmax_t>::max()
    == std::numeric_limits<json_int_t>::max()
    , "");

inline bool case_eq (string_view const & a, string_view const & b)
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

jansson::rep::rep (json_t * p) : basic_rep()
{
    _ptr = p;
}

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

jansson::ref::ref (ref const & other)
{
    if (other._ptr)
        _ptr = json_incref(other._ptr);

    if (other._parent) {
        _parent = json_incref(other._parent);

        if (json_is_object(_parent))
            new (& _index.key) key_type(std::move(other._index.key));
        else
            _index.i = other._index.i;
    }
}

jansson::ref::ref (ref && other)
{
    _ptr = other._ptr;
    _parent = other._parent;

    if (_parent) {
        if (json_is_object(_parent))
            new (& _index.key) key_type(std::move(other._index.key));
        else
            _index.i = other._index.i;
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
        _index.i = index;
    }
}

jansson::ref::ref (json_t * ptr, json_t * parent, std::string const & key)
{
    if (ptr)
        _ptr = json_incref(ptr);

    if (parent) {
        _parent = json_incref(parent);
        new (& _index.key) key_type(key);
    }
}

jansson::ref::~ref ()
{
    if (_parent) {
        if (json_is_object(_parent))
            _index.key.~basic_string();
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

// value must be a new reference
void assign (jansson::rep & rep, json_t * value)
{
    if (!value) {
        JEYSON__THROW((error{errc::invalid_argument
            , "Attempt to assign null value"}));
    }

    if (rep._ptr) {
        json_decref(rep._ptr);
        rep._ptr = nullptr;
    }

    rep._ptr = value;
}

// value must be a new reference
void assign (jansson::ref & ref, json_t * value)
{
    if (!value) {
        JEYSON__THROW((error{errc::invalid_argument
            , "Attempt to assign null value"}));
    }

    if (ref._ptr) {
        json_decref(ref._ptr);
        ref._ptr = nullptr;
    }

    if (ref._parent) {
        if (json_is_array(ref._parent)) {
            // Do not steal reference
            auto rc = json_array_set(ref._parent, ref._index.i, value);

            if (rc != 0) {
                JEYSON__THROW((error{
                      errc::backend_error
                    , "Replace array element failure"}));
            }

            // Not need `json_incref(value)`, we use non-steal variant of
            // `json_array_set()` function
            ref._ptr = value; // json_incref(ref._ptr);
        } else if (json_is_object(ref._parent)) {
            auto rc = json_object_setn_nocheck(ref._parent
                , ref._index.key.c_str()
                , ref._index.key.size()
                , value);

            if (rc != 0) {
                JEYSON__THROW((error{
                      errc::backend_error
                    , "Replace object element failure"}));
            }

            // Not need `json_incref(value)`, we use non-steal variant of
            // `json_object_setn_nocheck()` function
            ref._ptr = value;
        } else {
            JEYSON__THROW((error{
                   errc::incopatible_type
                , "Expected array or object for parent"}));
        }
    }
}

// value must be a new reference
void insert (json_t * obj, string_view const & key, json_t * value)
{
    if (!value) {
        JEYSON__THROW((error{errc::invalid_argument
            , "Attempt to insert null value"}));
    }

    if (!json_is_object(obj)) {
        error err{errc::incopatible_type, "Object expected"};
        JEYSON__THROW(err);
    }

    auto rc = json_object_setn_new_nocheck(obj, key.data(), key.size(), value);

    if (rc != 0) {
        error err{errc::backend_error, "object insertion failure"};
        JEYSON__THROW(err);
    }
}

// value must be a new reference
void push_back (json_t * arr, json_t * value)
{
    if (!value) {
        JEYSON__THROW((error{errc::invalid_argument
            , "Attempt to push back null value"}));
    }

    if (!json_is_array(arr)) {
        error err{errc::incopatible_type, "Array expected"};
        JEYSON__THROW(err);
    }

    auto rc = json_array_append_new(arr, value);

    if (rc != 0) {
        error err{errc::backend_error, "Array append failure"};
        JEYSON__THROW(err);
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
json<BACKEND>::json (string_view const & value)
    : rep_type(json_stringn_nocheck(value.data(), value.size()))
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

template <>
void
json<BACKEND>::assign_helper (std::nullptr_t)
{
    if (!json_is_null(_ptr))
        backend::assign(*this, json_null());
}

template <>
void
json<BACKEND>::assign_helper (bool b)
{
    backend::assign(*this, json_boolean(b));
}

template <>
void
json<BACKEND>::assign_helper (std::intmax_t n)
{
    if (!json_is_integer(_ptr))
        backend::assign(*this, json_integer(n));
    else
        json_integer_set(_ptr, n);
}

template <>
void
json<BACKEND>::assign_helper (double n)
{
    if (!json_is_real(_ptr))
        backend::assign(*this, json_real(n));
    else
        json_real_set(_ptr, n);
}

template <>
void
json<BACKEND>::assign_helper (string_view const & s)
{
    if (!json_is_string(_ptr))
        backend::assign(*this, json_stringn_nocheck(s.data(), s.size()));
    else
        json_string_setn_nocheck(_ptr, s.data(), s.size());
}

template <>
void
json<BACKEND>::assign_helper (json const & j)
{
    // FIXME
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
json_ref<BACKEND>::json_ref (json_ref const &) = default;

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

template <>
void
json_ref<BACKEND>::assign_helper (std::nullptr_t)
{
    backend::assign(*this, json_null());
}

template <>
void
json_ref<BACKEND>::assign_helper (bool b)
{
    backend::assign(*this, json_boolean(b));
}

template <>
void
json_ref<BACKEND>::assign_helper (std::intmax_t n)
{
    backend::assign(*this, json_integer(n));
}

template <>
void
json_ref<BACKEND>::assign_helper (double n)
{
    backend::assign(*this, json_real(n));
}

template <>
void
json_ref<BACKEND>::assign_helper (string_view const & s)
{
    backend::assign(*this, json_stringn_nocheck(s.data(), s.size()));
}

template <>
void
json_ref<BACKEND>::assign_helper (json<BACKEND> const & j)
{
    // FIXME
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
modifiers_interface<BACKEND>::insert_helper (string_view const & key
    , std::nullptr_t)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    backend::insert(INATIVE(*this), key, json_null());
}

template <>
void
modifiers_interface<BACKEND>::insert_helper (string_view const & key, bool b)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    backend::insert(INATIVE(*this), key, json_boolean(b));
}

template <>
void
modifiers_interface<BACKEND>::insert_helper (string_view const & key, std::intmax_t n)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    backend::insert(INATIVE(*this), key, json_integer(n));
}

template <>
void
modifiers_interface<BACKEND>::insert_helper (string_view const & key, double n)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    backend::insert(INATIVE(*this), key, json_real(n));
}

template <>
void
modifiers_interface<BACKEND>::insert_helper (string_view const & key, string_view const & s)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    backend::insert(INATIVE(*this), key, json_stringn_nocheck(s.data(), s.size()));
}

template <>
void
modifiers_interface<BACKEND>::insert_helper (string_view const & key
    , json<BACKEND> const & value)
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
        , key.data()
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
modifiers_interface<BACKEND>::push_back_helper (std::nullptr_t)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_array();

    backend::push_back(INATIVE(*this), json_null());
}

template <>
void
modifiers_interface<BACKEND>::push_back_helper (bool b)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    backend::push_back(INATIVE(*this), json_boolean(b));
}

template <>
void
modifiers_interface<BACKEND>::push_back_helper (std::intmax_t n)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    backend::push_back(INATIVE(*this), json_integer(n));
}

template <>
void
modifiers_interface<BACKEND>::push_back_helper (double n)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    backend::push_back(INATIVE(*this), json_real(n));
}

template <>
void
modifiers_interface<BACKEND>::push_back_helper (string_view const & s)
{
    if (!INATIVE(*this))
        INATIVE(*this) = json_object();

    backend::push_back(INATIVE(*this), json_stringn_nocheck(s.data(), s.size()));
}

template <>
void
modifiers_interface<BACKEND>::push_back_helper (json<BACKEND> const & value)
{
    if (!value) {
        error err {errc::invalid_argument, "Attempt to add unitialized value"};
        JEYSON__THROW(err);
    }

    if (! INATIVE(*this))
        INATIVE(*this) = json_array();

    if (!json_is_array(INATIVE(*this))) {
        error err{errc::incopatible_type, "Array expected"};
        JEYSON__THROW(err);
    }

    auto copy = json_deep_copy(NATIVE(value));

    if (!copy) {
        error err{errc::backend_error, "Deep copy failure"};
        JEYSON__THROW(err);
    }

    auto rc = json_array_append_new(INATIVE(*this), copy);

    if (rc != 0) {
        error err{errc::backend_error, "Array append failure"};
        JEYSON__THROW(err);
    }
}

template <>
void
modifiers_interface<BACKEND>::push_back (json<BACKEND> && value)
{
    if (!value) {
        error err {errc::invalid_argument, "Attempt to add unitialized value"};
        JEYSON__THROW(err);
    }

    if (! INATIVE(*this))
        INATIVE(*this) = json_array();

    if (!json_is_array(INATIVE(*this))) {
        error err{errc::incopatible_type, "Array expected"};
        JEYSON__THROW(err);
    }

    auto rc = json_array_append_new(INATIVE(*this), NATIVE(value));

    if (rc != 0) {
        error err{errc::backend_error, "Array append failure"};
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
            , json_dump_callback, & result, JSON_COMPACT | JSON_ENCODE_ANY);

        if (rc != 0) {
            error err{errc::backend_error, "stringification failure"};
            JEYSON__THROW(err);
        }
    }

    return result;
};

////////////////////////////////////////////////////////////////////////////////
// Encoder / Decoder
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
// nullptr_t
//------------------------------------------------------------------------------
template <>
std::nullptr_t
decoder<std::nullptr_t>::operator () () const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
decoder<std::nullptr_t>::operator () (std::nullptr_t, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
decoder<std::nullptr_t>::operator () (bool, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
decoder<std::nullptr_t>::operator () (std::intmax_t, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
decoder<std::nullptr_t>::operator () (double, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
decoder<std::nullptr_t>::operator () (string_view const &, bool *) const noexcept
{
    return nullptr;
}

template <>
std::nullptr_t
decoder<std::nullptr_t>::operator () (std::size_t, bool, bool *) const noexcept
{
    return nullptr;
}

//------------------------------------------------------------------------------
// bool
//------------------------------------------------------------------------------
template <>
bool
decoder<bool>::operator () () const noexcept
{
    return false;
}

template <>
bool
decoder<bool>::operator () (std::nullptr_t, bool *) const noexcept
{
    return false;
}

template <>
bool
decoder<bool>::operator () (bool v, bool *) const noexcept
{
    return v;
}

template <>
bool
decoder<bool>::operator () (std::intmax_t v, bool *) const noexcept
{
    return static_cast<bool>(v);
}

template <>
bool
decoder<bool>::operator () (double v, bool *) const noexcept
{
    return static_cast<bool>(v);
}

template <>
bool
decoder<bool>::operator () (string_view const & v, bool * success) const noexcept
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
decoder<bool>::operator () (std::size_t size, bool, bool *) const noexcept
{
    return size > 0;
}

//------------------------------------------------------------------------------
// std::intmax_t
//------------------------------------------------------------------------------
template <>
std::intmax_t
decoder<std::intmax_t>::operator () () const noexcept
{
    return 0;
}

template <>
std::intmax_t
decoder<std::intmax_t>::operator () (std::nullptr_t, bool *) const noexcept
{
    return 0;
}

template <>
std::intmax_t
decoder<std::intmax_t>::operator () (bool v, bool *) const noexcept
{
    return v ? 1 : 0;
}

template <>
std::intmax_t
decoder<std::intmax_t>::operator () (std::intmax_t v, bool *) const noexcept
{
    return v;
}

template <>
std::intmax_t
decoder<std::intmax_t>::operator () (double v, bool * success) const noexcept
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
decoder<std::intmax_t>::operator () (string_view const & v, bool * success) const noexcept
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
decoder<std::intmax_t>::operator () (std::size_t size, bool, bool *) const noexcept
{
    return static_cast<std::intmax_t>(size);
}

//------------------------------------------------------------------------------
// double
//------------------------------------------------------------------------------
template <>
double
decoder<double>::operator () () const noexcept
{
    return 0.0;
}

template <>
double
decoder<double>::operator () (std::nullptr_t, bool *) const noexcept
{
    return 0.0;
}

template <>
double
decoder<double>::operator () (bool v, bool *) const noexcept
{
    return v ? 1.0 : 0.0;
}

template <>
double
decoder<double>::operator () (std::intmax_t v, bool *) const noexcept
{
    return static_cast<double>(v);
}

template <>
double
decoder<double>::operator () (double v, bool *) const noexcept
{
    return v;
}

template <>
double
decoder<double>::operator () (string_view const & v, bool * success) const noexcept
{
    if (!v.empty()) {
        errno = 0;
        char * endptr = nullptr;
        auto n = std::strtod(v.data(), & endptr);

        auto ok = (endptr == (v.data() + v.size()) && !errno);

        if (ok)
            return n;
    }

    *success = false;
    return this->operator()();
}

template <>
double
decoder<double>::operator () (std::size_t size, bool, bool *) const noexcept
{
    return static_cast<std::intmax_t>(size);
}

//------------------------------------------------------------------------------
// std::string
//------------------------------------------------------------------------------
template <>
std::string
decoder<std::string>::operator () () const noexcept
{
    return "";
}

template <>
std::string
decoder<std::string>::operator () (std::nullptr_t, bool *) const noexcept
{
    return "";
}

template <>
std::string
decoder<std::string>::operator () (bool v, bool *) const noexcept
{
    return v ? "true" : "false";
}

template <>
std::string
decoder<std::string>::operator () (std::intmax_t v, bool *) const noexcept
{
    return std::to_string(v);
}

template <>
std::string
decoder<std::string>::operator () (double v, bool *) const noexcept
{
    return std::to_string(v);
}

template <>
std::string
decoder<std::string>::operator () (string_view const & v, bool *) const noexcept
{
    return std::string(v.data(), v.size());
}

template <>
std::string
decoder<std::string>::operator () (std::size_t size, bool, bool *) const noexcept
{
    return std::to_string(size);
}

////////////////////////////////////////////////////////////////////////////////
// Mutabl element accessor interface
////////////////////////////////////////////////////////////////////////////////
template <>
mutable_element_accessor_interface<BACKEND, true>::reference
mutable_element_accessor_interface<BACKEND, true>::operator [] (size_type pos) noexcept
{
    if (!INATIVE(*this))
        backend::assign(*reinterpret_cast<BACKEND::ref *>(this), json_array());

    if (json_is_null(INATIVE(*this)))
        backend::assign(*reinterpret_cast<BACKEND::ref *>(this), json_array());

    if (!json_is_array(INATIVE(*this)))
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
mutable_element_accessor_interface<BACKEND, true>::reference
mutable_element_accessor_interface<BACKEND, true>::operator [] (string_view const & key) noexcept
{
    if (!INATIVE(*this))
        backend::assign(*reinterpret_cast<BACKEND::ref *>(this), json_object());

    if (json_is_null(INATIVE(*this)))
        backend::assign(*reinterpret_cast<BACKEND::ref *>(this), json_object());

    if (!json_is_object(INATIVE(*this)))
        return reference{};

    // Return borrowed reference.
    auto ptr = json_object_getn(INATIVE(*this), key.data(), key.length());

    // Not found, insert new `null` element
    if (!ptr) {
        auto rc = json_object_setn_new_nocheck(INATIVE(*this)
            , key.data(), key.length(), json_null());

        if (rc != 0)
            return reference{};

        // Return borrowed reference.
        ptr = json_object_getn(INATIVE(*this), key.data(), key.length());

        JEYSON__ASSERT(ptr, "");
    }

    return reference{BACKEND::ref{ptr, INATIVE(*this), key_type(key.data(), key.length())}};
}

template <>
mutable_element_accessor_interface<BACKEND, false>::reference
mutable_element_accessor_interface<BACKEND, false>::operator [] (size_type pos) noexcept
{
    if (!INATIVE(*this))
        backend::assign(*reinterpret_cast<BACKEND::rep *>(this), json_array());

    if (json_is_null(INATIVE(*this)))
        backend::assign(*reinterpret_cast<BACKEND::rep *>(this), json_array());

    if (!json_is_array(INATIVE(*this)))
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
mutable_element_accessor_interface<BACKEND, false>::reference
mutable_element_accessor_interface<BACKEND, false>::operator [] (string_view const & key) noexcept
{
    if (!INATIVE(*this))
        backend::assign(*reinterpret_cast<BACKEND::rep *>(this), json_object());

    if (json_is_null(INATIVE(*this)))
        backend::assign(*reinterpret_cast<BACKEND::rep *>(this), json_object());

    if (!json_is_object(INATIVE(*this)))
        return reference{};

    // Return borrowed reference.
    auto ptr = json_object_getn(INATIVE(*this), key.data(), key.length());

    // Not found, insert new `null` element
    if (!ptr) {
        auto rc = json_object_setn_new_nocheck(INATIVE(*this)
            , key.data(), key.length(), json_null());

        if (rc != 0)
            return reference{};

        // Return borrowed reference.
        ptr = json_object_getn(INATIVE(*this), key.data(), key.length());

        JEYSON__ASSERT(ptr, "");
    }

    return reference{BACKEND::ref{ptr, INATIVE(*this), key_type(key.data(), key.length())}};
}

////////////////////////////////////////////////////////////////////////////////
// Element accessor interface
////////////////////////////////////////////////////////////////////////////////
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
element_accessor_interface<BACKEND>::const_reference
element_accessor_interface<BACKEND>::operator [] (string_view const & key) const noexcept
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
element_accessor_interface<BACKEND>::at (string_view const & key) const
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

////////////////////////////////////////////////////////////////////////////////
// Getter interface
////////////////////////////////////////////////////////////////////////////////
template <>
bool
getter_interface<BACKEND>::bool_value () const noexcept
{
    JEYSON__ASSERT(json_is_boolean(CINATIVE(*this)), "Boolean value expected");
    return json_is_true(CINATIVE(*this)) ? true : false;
}

template <>
std::intmax_t
getter_interface<BACKEND>::integer_value () const noexcept
{
    JEYSON__ASSERT(json_is_integer(CINATIVE(*this)), "Integer value expected");
    return static_cast<std::intmax_t>(json_integer_value(CINATIVE(*this)));
}

template <>
double
getter_interface<BACKEND>::real_value () const noexcept
{
    JEYSON__ASSERT(json_is_real(CINATIVE(*this)), "Real value expected");
    return json_real_value(CINATIVE(*this));
}

template <>
string_view
getter_interface<BACKEND>::string_value () const noexcept
{
    JEYSON__ASSERT(json_is_string(CINATIVE(*this)), "String value expected");
    return string_view(json_string_value(CINATIVE(*this))
        , json_string_length(CINATIVE(*this)));
}

template <>
std::size_t
getter_interface<BACKEND>::array_size () const noexcept
{
    JEYSON__ASSERT(json_is_array(CINATIVE(*this)), "Array expected");
    return json_array_size(CINATIVE(*this));
}

template <>
std::size_t
getter_interface<BACKEND>::object_size () const noexcept
{
    JEYSON__ASSERT(json_is_object(CINATIVE(*this)), "Object expected");
    return json_object_size(CINATIVE(*this));
}

} // namespace jeyson
