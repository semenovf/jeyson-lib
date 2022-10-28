////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
//      2022.07.08 Fixed for MSVC.
////////////////////////////////////////////////////////////////////////////////
#include "pfs/jeyson/json.hpp"
#include "pfs/jeyson/error.hpp"
#include "pfs/jeyson/backend/jansson.hpp"
#include "pfs/assert.hpp"
#include "pfs/fmt.hpp"
#include <jansson.h>
#include <algorithm>
#include <limits>
#include <sstream>
#include <cassert>
#include <cctype>
#include <cstdlib>

namespace jeyson {

using BACKEND  = backend::jansson;
using JSON     = json<BACKEND>;
using JSON_REF = json_ref<BACKEND>;

#define NATIVE(x) ((x)._ptr)
#define INATIVE(x) (reinterpret_cast<BACKEND::basic_rep *>(& x)->_ptr)
#define CINATIVE(x) (reinterpret_cast<BACKEND::basic_rep const *>(& x)->_ptr)

static_assert((std::numeric_limits<std::intmax_t>::max)()
    == (std::numeric_limits<json_int_t>::max)()
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
jansson::rep::rep () 
{}

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

inline void swap (jansson::rep & a, jansson::rep & b)
{
    using std::swap;
    swap(a._ptr, b._ptr);
}

////////////////////////////////////////////////////////////////////////////////
// ref
////////////////////////////////////////////////////////////////////////////////
jansson::ref::ref ()
{}

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

void swap (jansson::ref & a, jansson::ref & b)
{
    using std::swap;

    if (json_is_array(a._parent)) {
        // Both are arrays
        if (json_is_array(b._parent)) {
            swap(a._index.i, b._index.i);
        } else {
            auto i = a._index.i;
            new (& a._index.key) std::string(std::move(b._index.key));
            b._index.key.~basic_string();
            b._index.i = i;
        }
    } else {
        // Both are objects
        if (json_is_object(b._parent)) {
            swap(a._index.key, b._index.key);
        } else {
            auto i = b._index.i;
            new (& b._index.key) std::string(std::move(a._index.key));
            a._index.key.~basic_string();
            a._index.i = i;
        }
    }

    swap(a._ptr, b._ptr);
    swap(a._parent, b._parent);
}

// value must be a new reference
void assign (jansson::rep & rep, json_t * value)
{
    if (!value)
        throw error {errc::invalid_argument, "attempt to assign null value"};

    if (rep._ptr) {
        json_decref(rep._ptr);
        rep._ptr = nullptr;
    }

    rep._ptr = value;
}

// value must be a new reference
void assign (jansson::ref & ref, json_t * value)
{
    if (!value)
        throw error {errc::invalid_argument, "attempt to assign null value"};

    if (ref._ptr) {
        json_decref(ref._ptr);
        ref._ptr = nullptr;
    }

    if (ref._parent) {
        if (json_is_array(ref._parent)) {
            // Do not steal reference
            auto rc = json_array_set(ref._parent, ref._index.i, value);

            if (rc != 0)
                throw error {errc::backend_error, "replace array element failure"};

            // Not need `json_incref(value)`, we use non-steal variant of
            // `json_array_set()` function
            ref._ptr = value; // json_incref(ref._ptr);
        } else if (json_is_object(ref._parent)) {
            auto rc = json_object_setn_nocheck(ref._parent
                , ref._index.key.c_str()
                , ref._index.key.size()
                , value);

            if (rc != 0)
                throw error {errc::backend_error, "replace object element failure"};

            // Not need `json_incref(value)`, we use non-steal variant of
            // `json_object_setn_nocheck()` function
            ref._ptr = value;
        } else {
            throw error {errc::incopatible_type, "array or object expected for parent"};
        }
    } else {
        ref._ptr = value;
    }
}

// value must be a new reference
void insert (json_t * obj, string_view const & key, json_t * value)
{
    if (!value)
        throw error {errc::invalid_argument, "attempt to insert null value"};

    if (!json_is_object(obj))
        throw error {errc::incopatible_type, "object expected"};

    auto rc = json_object_setn_new_nocheck(obj, key.data(), key.size(), value);

    if (rc != 0)
        throw error {errc::backend_error, "object insertion failure"};
}

// value must be a new reference
void push_back (json_t * arr, json_t * value)
{
    if (!value)
        throw error {errc::invalid_argument
            , "attempt to push back null value"};

    if (!json_is_array(arr))
        throw error {errc::incopatible_type, "array expected"};

    auto rc = json_array_append_new(arr, value);

    if (rc != 0)
        throw error {errc::backend_error, "array append failure"};
}

} // namespace backend

////////////////////////////////////////////////////////////////////////////////
// JSON reference destructor
////////////////////////////////////////////////////////////////////////////////
// Placed here to avoid error: specialization of ‘jeyson::json_ref<Backend>::~json_ref()
// [with Backend = jeyson::backend::jansson]’ after instantiation
template <>
json_ref<BACKEND>::~json_ref ()
{}

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
json<BACKEND>::json ()
{}

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

// NOTE. MSVC 2019 (Checked with version 16.11.16):
// This code
// 
// template <>
// json<BACKEND>::json (json const & other) = default;
// 
// is the cause of the error LNK2019: unresolved external symbol

template <>
json<BACKEND>::json (json const & other)
    : rep_type(other)
{}

template <>
json<BACKEND>::json (json && other)
    : rep_type(std::move(other))
{}

template <>
json<BACKEND>::json (json_ref<BACKEND> const & j)
{
    backend::assign(*this, json_deep_copy(j._ptr));
}

template <>
json<BACKEND>::json (json_ref<BACKEND> && j)
{
    backend::assign(*this, json_deep_copy(j._ptr));
    j.~json_ref<BACKEND>();
}

template <>
json<BACKEND>::~json ()
{}

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
json<BACKEND> &
json<BACKEND>::operator = (json_ref<BACKEND> const & j)
{
    backend::assign(*this, json_deep_copy(j._ptr));
    return *this;
}

template <>
json<BACKEND> &
json<BACKEND>::operator = (json_ref<BACKEND> && j)
{
    backend::assign(*this, json_deep_copy(j._ptr));
    j.~json_ref<BACKEND>();
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

//------------------------------------------------------------------------------
// Modifiers
//------------------------------------------------------------------------------
template <>
void
json<BACKEND>::swap (json & other)
{
    backend::swap(*this, other);
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
        throw error {
              errc::backend_error
            , fmt::format("save JSON representation to file `{}` failure"
                , pfs::filesystem::utf8_encode(path))
        };
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
        throw error {
              errc::backend_error
            , fmt::format("parse error at line {}", jerror.line)
            , jerror.text
        };
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
        throw error {
              errc::backend_error
            , fmt::format("parse error at line {} in file `{}`"
                , jerror.line
                , pfs::filesystem::utf8_encode(path))
            , jerror.text};
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
json_ref<BACKEND>::json_ref ()
{}

template <>
json_ref<BACKEND>::json_ref (json_ref const & other)
    : rep_type(other)
{}

template <>
json_ref<BACKEND>::json_ref (json_ref && other)
    : rep_type(std::move(other))
{}

template <>
json_ref<BACKEND>::json_ref (BACKEND::ref && other)
    : BACKEND::ref(std::move(other))
{}

template <>
json_ref<BACKEND>::json_ref (json<BACKEND> & j)
{
    if (j._ptr)
        backend::assign(*this, json_incref(j._ptr));
}

template <>
json_ref<BACKEND>::json_ref (json<BACKEND> && j)
{
    if (j._ptr) {
        backend::assign(*this, json_incref(j._ptr));
        j.~json<BACKEND>();
    }
}

template <>
json_ref<BACKEND> &
json_ref<BACKEND>::operator = (json_ref const & j)
{
    if (j._ptr)
        backend::assign(*this, json_deep_copy(j._ptr));

    return *this;
}

template <>
json_ref<BACKEND> &
json_ref<BACKEND>::operator = (json_ref && j)
{
    if (j._ptr) {
        backend::assign(*this, json_deep_copy(j._ptr));
        j.~json_ref();
    }

    return *this;
}

template <>
json_ref<BACKEND> & json_ref<BACKEND>::operator = (json<BACKEND> const & j)
{
    if (j._ptr)
        backend::assign(*this, json_deep_copy(j._ptr));

    return *this;
}

template <>
json_ref<BACKEND> & json_ref<BACKEND>::operator = (json<BACKEND> && j)
{
    if (j._ptr) {
        backend::assign(*this, json_deep_copy(j._ptr));
        j.~json<BACKEND>();
    }

    return *this;
}

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

//------------------------------------------------------------------------------
// Modifiers
//------------------------------------------------------------------------------
template <>
void
json_ref<BACKEND>::swap (json_ref & other)
{
    backend::swap(*this, other);
}


////////////////////////////////////////////////////////////////////////////////
// Traits interface
////////////////////////////////////////////////////////////////////////////////
template <typename Derived>
bool traits_interface<Derived>::is_null () const noexcept
{
    auto self = static_cast<Derived const *>(this);
    return CINATIVE(*self) ? json_is_null(CINATIVE(*self)) : false;
}

template bool traits_interface<JSON>::is_null() const noexcept;
template bool traits_interface<JSON_REF>::is_null() const noexcept;

template <typename Derived>
bool
traits_interface<Derived>::is_bool () const noexcept
{
    auto self = static_cast<Derived const*>(this);
    return CINATIVE(*self) ? json_is_boolean(CINATIVE(*self)) : false;
}

template bool traits_interface<JSON>::is_bool() const noexcept;
template bool traits_interface<JSON_REF>::is_bool() const noexcept;

template <typename Derived>
bool
traits_interface<Derived>::is_integer () const noexcept
{
    auto self = static_cast<Derived const *>(this);
    return CINATIVE(*self) ? json_is_integer(CINATIVE(*self)) : false;
}

template bool traits_interface<JSON>::is_integer() const noexcept;
template bool traits_interface<JSON_REF>::is_integer() const noexcept;

template <typename Derived>
bool
traits_interface<Derived>::is_real () const noexcept
{
    auto self = static_cast<Derived const *>(this);
    return CINATIVE(*self) ? json_is_real(CINATIVE(*self)) : false;
}

template bool traits_interface<JSON>::is_real() const noexcept;
template bool traits_interface<JSON_REF>::is_real() const noexcept;

template <typename Derived>
bool
traits_interface<Derived>::is_string () const noexcept
{
    auto self = static_cast<Derived const *>(this);
    return CINATIVE(*self) ? json_is_string(CINATIVE(*self)) : false;
}

template bool traits_interface<JSON>::is_string() const noexcept;
template bool traits_interface<JSON_REF>::is_string() const noexcept;

template <typename Derived>
bool
traits_interface<Derived>::is_array () const noexcept
{
    auto self = static_cast<Derived const *>(this);
    return CINATIVE(*self) ? json_is_array(CINATIVE(*self)) : false;
}

template bool traits_interface<JSON>::is_array() const noexcept;
template bool traits_interface<JSON_REF>::is_array() const noexcept;

template <typename Derived>
bool
traits_interface<Derived>::is_object () const noexcept
{
    auto self = static_cast<Derived const *>(this);
    return CINATIVE(*self) ? json_is_object(CINATIVE(*self)) : false;
}

template bool traits_interface<JSON>::is_object() const noexcept;
template bool traits_interface<JSON_REF>::is_object() const noexcept;

////////////////////////////////////////////////////////////////////////////////
// Modifiers interface
////////////////////////////////////////////////////////////////////////////////
template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::insert_helper (string_view const & key
    , std::nullptr_t)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    backend::insert(INATIVE(*self), key, json_null());
}
template void modifiers_interface<JSON, BACKEND>::insert_helper (string_view const &, std::nullptr_t);
template void modifiers_interface<JSON_REF, BACKEND>::insert_helper (string_view const &, std::nullptr_t);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::insert_helper (string_view const & key, bool b)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    backend::insert(INATIVE(*self), key, json_boolean(b));
}

template void modifiers_interface<JSON, BACKEND>::insert_helper (string_view const &, bool);
template void modifiers_interface<JSON_REF, BACKEND>::insert_helper (string_view const &, bool);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::insert_helper (string_view const & key, std::intmax_t n)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    backend::insert(INATIVE(*self), key, json_integer(n));
}

template void modifiers_interface<JSON, BACKEND>::insert_helper (string_view const &, std::intmax_t);
template void modifiers_interface<JSON_REF, BACKEND>::insert_helper (string_view const &, std::intmax_t);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::insert_helper (string_view const & key, double n)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    backend::insert(INATIVE(*self), key, json_real(n));
}

template void modifiers_interface<JSON, BACKEND>::insert_helper (string_view const &, double);
template void modifiers_interface<JSON_REF, BACKEND>::insert_helper (string_view const &, double);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::insert_helper (string_view const & key, string_view const & s)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    backend::insert(INATIVE(*self), key, json_stringn_nocheck(s.data(), s.size()));
}

template void modifiers_interface<JSON, BACKEND>::insert_helper (string_view const &, string_view const &);
template void modifiers_interface<JSON_REF, BACKEND>::insert_helper (string_view const &, string_view const &);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::insert_helper (string_view const & key
    , value_type const & value)
{
    auto self = static_cast<Derived *>(this);

    if (!value)
        throw error {errc::invalid_argument, "attempt to insert unitialized value"};

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    if (!json_is_object(INATIVE(*self)))
        throw error {errc::incopatible_type, "object expected"};

    auto copy = json_deep_copy(NATIVE(value));

    if (!copy)
        throw error {errc::backend_error, "deep copy failure"};

    auto rc = json_object_setn_new_nocheck(INATIVE(*self)
        , key.data()
        , key.size()
        , copy);

    if (rc != 0)
        throw error {errc::backend_error, "object insertion failure"};
}

template void modifiers_interface<JSON, BACKEND>::insert_helper (string_view const &, value_type const &);
template void modifiers_interface<JSON_REF, BACKEND>::insert_helper (string_view const &, value_type const &);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::insert (key_type const & key, value_type && value)
{
    auto self = static_cast<Derived *>(this);

    if (!value)
        throw error {errc::invalid_argument, "attempt to insert unitialized value"};

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    if (!json_is_object(INATIVE(*self)))
        throw error {errc::incopatible_type, "object expected"};

    auto rc = json_object_setn_new_nocheck(INATIVE(*self)
        , key.c_str()
        , key.size()
        , NATIVE(value));

    if (rc != 0)
        throw error {errc::backend_error, "object insertion failure"};

    NATIVE(value) = nullptr;
}

template void modifiers_interface<JSON, BACKEND>::insert (key_type const &, value_type &&);
template void modifiers_interface<JSON_REF, BACKEND>::insert (key_type const &, value_type &&);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::push_back_helper (std::nullptr_t)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_array();

    backend::push_back(INATIVE(*self), json_null());
}

template void modifiers_interface<JSON, BACKEND>::push_back_helper (std::nullptr_t);
template void modifiers_interface<JSON_REF, BACKEND>::push_back_helper (std::nullptr_t);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::push_back_helper (bool b)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    backend::push_back(INATIVE(*self), json_boolean(b));
}

template void modifiers_interface<JSON, BACKEND>::push_back_helper (bool);
template void modifiers_interface<JSON_REF, BACKEND>::push_back_helper (bool);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::push_back_helper (std::intmax_t n)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    backend::push_back(INATIVE(*self), json_integer(n));
}

template void modifiers_interface<JSON, BACKEND>::push_back_helper (std::intmax_t);
template void modifiers_interface<JSON_REF, BACKEND>::push_back_helper (std::intmax_t);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::push_back_helper (double n)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    backend::push_back(INATIVE(*self), json_real(n));
}

template void modifiers_interface<JSON, BACKEND>::push_back_helper (double);
template void modifiers_interface<JSON_REF, BACKEND>::push_back_helper (double);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::push_back_helper (string_view const & s)
{
    auto self = static_cast<Derived *>(this);

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    backend::push_back(INATIVE(*self), json_stringn_nocheck(s.data(), s.size()));
}

template void modifiers_interface<JSON, BACKEND>::push_back_helper (string_view const &);
template void modifiers_interface<JSON_REF, BACKEND>::push_back_helper (string_view const &);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::push_back_helper (value_type const & value)
{
    auto self = static_cast<Derived *>(this);

    if (!value)
        throw error {errc::invalid_argument, "attempt to add unitialized value"};

    if (! INATIVE(*self))
        INATIVE(*self) = json_array();

    if (!json_is_array(INATIVE(*self)))
        throw error {errc::incopatible_type, "array expected"};

    auto copy = json_deep_copy(NATIVE(value));

    if (!copy)
        throw error {errc::backend_error, "deep copy failure"};

    auto rc = json_array_append_new(INATIVE(*self), copy);

    if (rc != 0)
        throw error {errc::backend_error, "array append failure"};
}

template void modifiers_interface<JSON, BACKEND>::push_back_helper (value_type const &);
template void modifiers_interface<JSON_REF, BACKEND>::push_back_helper (value_type const &);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::push_back (value_type && value)
{
    auto self = static_cast<Derived *>(this);

    if (!value)
        throw error {errc::invalid_argument, "attempt to add unitialized value"};

    if (! INATIVE(*self))
        INATIVE(*self) = json_array();

    if (!json_is_array(INATIVE(*self)))
        throw error {errc::incopatible_type, "array expected"};

    auto rc = json_array_append_new(INATIVE(*self), NATIVE(value));

    if (rc != 0)
        throw error {errc::backend_error, "array append failure"};

    NATIVE(value) = nullptr;
}

template void modifiers_interface<JSON, BACKEND>::push_back (value_type &&);
template void modifiers_interface<JSON_REF, BACKEND>::push_back (value_type &&);

////////////////////////////////////////////////////////////////////////////////
// Capacity interface
////////////////////////////////////////////////////////////////////////////////
template <typename Derived, typename Backend>
typename capacity_interface<Derived, Backend>::size_type
capacity_interface<Derived, Backend>::size () const noexcept
{
    auto self = static_cast<Derived const *>(this);

    if (!CINATIVE(*self))
        return 0;

    if (json_is_object(CINATIVE(*self)))
        return json_object_size(CINATIVE(*self));

    if (json_is_array(CINATIVE(*self)))
        return json_array_size(CINATIVE(*self));

    // Scalar types
    return 1;
}

template capacity_interface<JSON, BACKEND>::size_type capacity_interface<JSON, BACKEND>::size () const noexcept;
template capacity_interface<JSON_REF, BACKEND>::size_type capacity_interface<JSON_REF, BACKEND>::size () const noexcept;

////////////////////////////////////////////////////////////////////////////////
// Converter interface
////////////////////////////////////////////////////////////////////////////////
static int json_dump_callback (char const * buffer, size_t size, void * data)
{
    auto output = static_cast<std::string *>(data);
    *output += std::string(buffer, size);
    return 0;
}

template <typename Derived>
std::string
converter_interface<Derived>::to_string () const
{
    std::string result;

    auto self = static_cast<Derived const *>(this);

    if (CINATIVE(*self)) {
        auto rc = json_dump_callback(CINATIVE(*self)
            , json_dump_callback, & result, JSON_COMPACT | JSON_ENCODE_ANY);

        if (rc != 0)
            throw error {errc::backend_error, "stringification failure"};
    }

    return result;
};

template std::string converter_interface<JSON>::to_string () const;
template std::string converter_interface<JSON_REF>::to_string () const;

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
    if (v >= static_cast<double>((std::numeric_limits<std::intmax_t>::min)())
            && v <= static_cast<double>((std::numeric_limits<std::intmax_t>::max)())) {
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
    return static_cast<double>(size);
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
// Mutable element accessor interface
////////////////////////////////////////////////////////////////////////////////
template <>
mutable_element_accessor_interface<JSON, BACKEND>::reference
mutable_element_accessor_interface<JSON, BACKEND>::operator [] (size_type pos) noexcept
{
    auto self = static_cast<JSON *>(this);

    if (!INATIVE(*self))
        backend::assign(*static_cast<BACKEND::rep *>(self), json_array());

    if (json_is_null(INATIVE(*self)))
        backend::assign(*static_cast<BACKEND::rep *>(self), json_array());

    if (!json_is_array(INATIVE(*self)))
        return reference{};

    if (pos >= json_array_size(INATIVE(*self))) {
        // Fill with null values
        while (json_array_size(INATIVE(*self)) <= pos) {
            auto rc = json_array_append_new(INATIVE(*self), json_null());
            PFS__ASSERT(rc == 0, "json_array_append_new() failure");
        }
    }

    auto ptr = json_array_get(INATIVE(*self), pos);

    if (!ptr)
        return reference{};

    return reference{BACKEND::ref{ptr, INATIVE(*self), pos}};
}

template <>
mutable_element_accessor_interface<JSON, BACKEND>::reference
mutable_element_accessor_interface<JSON, BACKEND>::operator [] (string_view key) noexcept
{
    auto self = static_cast<JSON *>(this);

    if (!INATIVE(*self))
        backend::assign(*static_cast<BACKEND::rep *>(self), json_object());

    if (json_is_null(INATIVE(*self)))
        backend::assign(*static_cast<BACKEND::rep *>(self), json_object());

    if (!json_is_object(INATIVE(*self)))
        return reference{};

    // Return borrowed reference.
    auto ptr = json_object_getn(INATIVE(*self), key.data(), key.length());

    // Not found, insert new `null` element
    if (!ptr) {
        auto rc = json_object_setn_new_nocheck(INATIVE(*self)
            , key.data(), key.length(), json_null());

        if (rc != 0)
            return reference{};

        // Return borrowed reference.
        ptr = json_object_getn(INATIVE(*self), key.data(), key.length());

        PFS__ASSERT(ptr, "");
    }

    return reference{BACKEND::ref{ptr, INATIVE(*self), key_type(key.data(), key.length())}};
}

template <>
mutable_element_accessor_interface<JSON_REF, BACKEND>::reference
mutable_element_accessor_interface<JSON_REF, BACKEND>::operator [] (size_type pos) noexcept
{
    auto self = static_cast<JSON_REF *>(this);

    if (!INATIVE(*self))
        backend::assign(*static_cast<BACKEND::ref *>(self), json_array());

    if (json_is_null(INATIVE(*self)))
        backend::assign(*static_cast<BACKEND::ref *>(self), json_array());

    if (!json_is_array(INATIVE(*self)))
        return reference{};

    if (pos >= json_array_size(INATIVE(*self))) {
        // Fill with null values
        while (json_array_size(INATIVE(*self)) <= pos) {
            auto rc = json_array_append_new(INATIVE(*self), json_null());
            PFS__ASSERT(rc == 0, "json_array_append_new() failure");
        }
    }

    auto ptr = json_array_get(INATIVE(*self), pos);

    if (!ptr)
        return reference{};

    return reference{BACKEND::ref{ptr, INATIVE(*self), pos}};
}

template <>
mutable_element_accessor_interface<JSON_REF, BACKEND>::reference
mutable_element_accessor_interface<JSON_REF, BACKEND>::operator [] (string_view key) noexcept
{
    auto self = static_cast<JSON_REF *>(this);

    if (!INATIVE(*self))
        backend::assign(*static_cast<BACKEND::ref *>(self), json_object());

    if (json_is_null(INATIVE(*self)))
        backend::assign(*static_cast<BACKEND::ref *>(self), json_object());

    if (!json_is_object(INATIVE(*self)))
        return reference{};

    // Return borrowed reference.
    auto ptr = json_object_getn(INATIVE(*self), key.data(), key.length());

    // Not found, insert new `null` element
    if (!ptr) {
        auto rc = json_object_setn_new_nocheck(INATIVE(*self)
            , key.data(), key.length(), json_null());

        if (rc != 0)
            return reference{};

        // Return borrowed reference.
        ptr = json_object_getn(INATIVE(*self), key.data(), key.length());

        PFS__ASSERT(ptr, "");
    }

    return reference{BACKEND::ref{ptr, INATIVE(*self), key_type(key.data(), key.length())}};
}

////////////////////////////////////////////////////////////////////////////////
// Element accessor interface
////////////////////////////////////////////////////////////////////////////////

template <typename Derived, typename Backend>
// Specify an explicit return type to avoid:
// error C2244: 'jeyson::element_accessor_interface<Derived,Backend>::operator []':
// unable to match function definition to an existing declaration
//typename element_accessor_interface<Derived, Backend>::const_reference
json_ref<Backend> const
element_accessor_interface<Derived, Backend>::operator [] (size_type pos) const noexcept
{
    auto self = static_cast<Derived const *>(this);

    if (!CINATIVE(*self) || !json_is_array(CINATIVE(*self)))
        return const_reference{};

    auto ptr = json_array_get(CINATIVE(*self), pos);

    if (!ptr)
        return const_reference{};

    return const_reference{BACKEND::ref{ptr, CINATIVE(*self), pos}};
}

template element_accessor_interface<JSON, BACKEND>::const_reference element_accessor_interface<JSON, BACKEND>::operator [] (size_type pos) const noexcept;
template element_accessor_interface<JSON_REF, BACKEND>::const_reference element_accessor_interface<JSON_REF, BACKEND>::operator [] (size_type pos) const noexcept;

template <typename Derived, typename Backend>
//typename element_accessor_interface<Derived, Backend>::const_reference
json_ref<Backend> const
element_accessor_interface<Derived, Backend>::operator [] (string_view key) const noexcept
{
    auto self = static_cast<Derived const *>(this);

    if (!CINATIVE(*self) || !json_is_object(CINATIVE(*self)))
        return reference{};

    auto ptr = json_object_getn(CINATIVE(*self), key.data(), key.length());

    // Not found
    if (!ptr)
        return reference{};

    return reference{BACKEND::ref{ptr, CINATIVE(*self), key_type(key.data(), key.length())}};
}

template element_accessor_interface<JSON, BACKEND>::const_reference element_accessor_interface<JSON, BACKEND>::operator [] (string_view key) const noexcept;
template element_accessor_interface<JSON_REF, BACKEND>::const_reference element_accessor_interface<JSON_REF, BACKEND>::operator [] (string_view key) const noexcept;

template <typename Derived, typename Backend>
typename element_accessor_interface<Derived, Backend>::reference
element_accessor_interface<Derived, Backend>::at (size_type pos) const
{
    auto self = static_cast<Derived const *>(this);

    if (!CINATIVE(*self) || !json_is_array(CINATIVE(*self)))
        throw error {errc::incopatible_type, "array expected"};

    auto ptr = json_array_get(CINATIVE(*self), pos);

    if (!ptr)
        throw error {errc::out_of_range};

    return reference{BACKEND::ref{ptr, CINATIVE(*self), pos}};
}

template element_accessor_interface<JSON, BACKEND>::reference element_accessor_interface<JSON, BACKEND>::at (size_type pos) const;
template element_accessor_interface<JSON_REF, BACKEND>::reference element_accessor_interface<JSON_REF, BACKEND>::at (size_type pos) const;

template <typename Derived, typename Backend>
typename element_accessor_interface<Derived, Backend>::reference
element_accessor_interface<Derived, Backend>::at (string_view key) const
{
    auto self = static_cast<Derived const *>(this);

    if (!CINATIVE(*self) || !json_is_object(CINATIVE(*self)))
        throw error {errc::incopatible_type, "object expected"};

    auto ptr = json_object_getn(CINATIVE(*self), key.data(), key.length());

    if (!ptr)
        throw error {errc::out_of_range};

    return reference{BACKEND::ref{ptr, CINATIVE(*self)
        , key_type(key.data(), key.length())}};
}

template element_accessor_interface<JSON, BACKEND>::reference element_accessor_interface<JSON, BACKEND>::at (string_view key) const;
template element_accessor_interface<JSON_REF, BACKEND>::reference element_accessor_interface<JSON_REF, BACKEND>::at (string_view key) const;

template <typename Derived, typename Backend>
bool
element_accessor_interface<Derived, Backend>::contains (string_view key) const
{
    auto self = static_cast<Derived const *>(this);

    if (!CINATIVE(*self) || !json_is_object(CINATIVE(*self)))
        return false;

    auto ptr = json_object_getn(CINATIVE(*self), key.data(), key.length());
    return !!ptr;
}

template bool element_accessor_interface<JSON, BACKEND>::contains (string_view key) const;
template bool element_accessor_interface<JSON_REF, BACKEND>::contains (string_view key) const;

////////////////////////////////////////////////////////////////////////////////
// Getter interface
////////////////////////////////////////////////////////////////////////////////
template <typename Derived, typename Backend>
bool
getter_interface<Derived, Backend>::bool_value () const noexcept
{
    auto self = static_cast<Derived const *>(this);

    PFS__ASSERT(json_is_boolean(CINATIVE(*self)), "boolean value expected");
    return json_is_true(CINATIVE(*self)) ? true : false;
}

template bool getter_interface<JSON, BACKEND>::bool_value () const noexcept;
template bool getter_interface<JSON_REF, BACKEND>::bool_value () const noexcept;

template <typename Derived, typename Backend>
std::intmax_t
getter_interface<Derived, Backend>::integer_value () const noexcept
{
    auto self = static_cast<Derived const *>(this);

    PFS__ASSERT(json_is_integer(CINATIVE(*self)), "integer value expected");
    return static_cast<std::intmax_t>(json_integer_value(CINATIVE(*self)));
}

template std::intmax_t getter_interface<JSON, BACKEND>::integer_value () const noexcept;
template std::intmax_t getter_interface<JSON_REF, BACKEND>::integer_value () const noexcept;

template <typename Derived, typename Backend>
double
getter_interface<Derived, Backend>::real_value () const noexcept
{
    auto self = static_cast<Derived const *>(this);

    PFS__ASSERT(json_is_real(CINATIVE(*self)), "real value expected");
    return json_real_value(CINATIVE(*self));
}

template double getter_interface<JSON, BACKEND>::real_value () const noexcept;
template double getter_interface<JSON_REF, BACKEND>::real_value () const noexcept;

template <typename Derived, typename Backend>
string_view
getter_interface<Derived, Backend>::string_value () const noexcept
{
    auto self = static_cast<Derived const *>(this);

    PFS__ASSERT(json_is_string(CINATIVE(*self)), "string value expected");
    return string_view(json_string_value(CINATIVE(*self))
        , json_string_length(CINATIVE(*self)));
}

template string_view getter_interface<JSON, BACKEND>::string_value () const noexcept;
template string_view getter_interface<JSON_REF, BACKEND>::string_value () const noexcept;

template <typename Derived, typename Backend>
std::size_t
getter_interface<Derived, Backend>::array_size () const noexcept
{
    auto self = static_cast<Derived const *>(this);

    PFS__ASSERT(json_is_array(CINATIVE(*self)), "array expected");
    return json_array_size(CINATIVE(*self));
}

template std::size_t getter_interface<JSON, BACKEND>::array_size () const noexcept;
template std::size_t getter_interface<JSON_REF, BACKEND>::array_size () const noexcept;

template <typename Derived, typename Backend>
std::size_t
getter_interface<Derived, Backend>::object_size () const noexcept
{
    auto self = static_cast<Derived const *>(this);

    PFS__ASSERT(json_is_object(CINATIVE(*self)), "object expected");
    return json_object_size(CINATIVE(*self));
}

template std::size_t getter_interface<JSON, BACKEND>::object_size () const noexcept;
template std::size_t getter_interface<JSON_REF, BACKEND>::object_size () const noexcept;

////////////////////////////////////////////////////////////////////////////////
// Algorithm interface
////////////////////////////////////////////////////////////////////////////////
template <typename Derived, typename Backend>
void
algoritm_interface<Derived, Backend>::for_each (std::function<void (reference)> f) const noexcept
{
    auto self = static_cast<Derived const *>(this);

    if (!CINATIVE(*self))
        return;

    if (json_is_object(CINATIVE(*self))) {
        char const * key;
        json_t * ptr;

        json_object_foreach (CINATIVE(*self), key, ptr) {
            f(reference{
                typename Backend::ref{
                      ptr
                    , CINATIVE(*self)
                    , std::string(key)
                }
            });
        }
    } else if (json_is_array(CINATIVE(*self))) {
        std::size_t index;
        json_t * ptr;

        json_array_foreach(CINATIVE(*self), index, ptr) {
            f(reference{
                typename Backend::ref{
                      ptr
                    , CINATIVE(*self)
                    , index
                }
            });
        }
    }
}

template void algoritm_interface<JSON, BACKEND>::for_each (std::function<void (reference)> f) const noexcept;
template void algoritm_interface<JSON_REF, BACKEND>::for_each (std::function<void (reference)> f) const noexcept;

} // namespace jeyson
