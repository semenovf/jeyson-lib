////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
//      2022.07.08 Fixed for MSVC.
////////////////////////////////////////////////////////////////////////////////
#include "jeyson/json.hpp"
#include "jeyson/error.hpp"
#include "jeyson/backend/jansson.hpp"
#include <pfs/assert.hpp>
#include <pfs/i18n.hpp>
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
        throw error {make_error_code(std::errc::invalid_argument), tr::_("attempt to assign null value")};

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
        throw error {make_error_code(std::errc::invalid_argument), tr::_("attempt to assign null value")};

    if (ref._ptr) {
        json_decref(ref._ptr);
        ref._ptr = nullptr;
    }

    if (ref._parent) {
        if (json_is_array(ref._parent)) {
            // Do not steal reference
            auto rc = json_array_set(ref._parent, ref._index.i, value);

            if (rc != 0)
                throw error {make_error_code(pfs::errc::backend_error), tr::_("replace array element failure")};

            // Not need `json_incref(value)`, we use non-steal variant of
            // `json_array_set()` function
            ref._ptr = value; // json_incref(ref._ptr);
        } else if (json_is_object(ref._parent)) {
            auto rc = json_object_setn_nocheck(ref._parent
                , ref._index.key.c_str()
                , ref._index.key.size()
                , value);

            if (rc != 0)
                throw error {make_error_code(pfs::errc::backend_error), tr::_("replace object element failure")};

            // Not need `json_incref(value)`, we use non-steal variant of
            // `json_object_setn_nocheck()` function
            ref._ptr = value;
        } else {
            throw error {make_error_code(errc::incopatible_type), tr::_("array or object expected for parent")};
        }
    } else {
        ref._ptr = value;
    }
}

// value must be a new reference
void insert (json_t * obj, string_view const & key, json_t * value)
{
    if (!value)
        throw error {make_error_code(std::errc::invalid_argument), tr::_("attempt to insert null value")};

    if (!json_is_object(obj))
        throw error {make_error_code(errc::incopatible_type), tr::_("object expected")};

    auto rc = json_object_setn_new_nocheck(obj, key.data(), key.size(), value);

    if (rc != 0)
        throw error {make_error_code(pfs::errc::backend_error), tr::_("object insertion failure")};
}

// value must be a new reference
void push_back (json_t * arr, json_t * value)
{
    if (!value)
        throw error {make_error_code(std::errc::invalid_argument), tr::_("attempt to push back null value")};

    if (!json_is_array(arr))
        throw error {make_error_code(errc::incopatible_type), tr::_("array expected")};

    auto rc = json_array_append_new(arr, value);

    if (rc != 0)
        throw error {make_error_code(pfs::errc::backend_error), tr::_("array append failure")};
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
        , pfs::utf8_encode_path(path).c_str()
        , flags);

    if (rc < 0) {
        throw error {
              make_error_code(pfs::errc::backend_error)
            , tr::f_("save JSON representation to file failure: {}", pfs::utf8_encode_path(path))
        };
    }
}

//------------------------------------------------------------------------------
// Parsing
//------------------------------------------------------------------------------
template <>
json<BACKEND>
json<BACKEND>::parse (char const * source, std::size_t len, error * perr)
{
    json_error_t jerror;

    auto j = json_loadb(source, len, JSON_DECODE_ANY, & jerror);

    if (!j) {
        pfs::throw_or(perr, make_error_code(pfs::errc::backend_error)
            , tr::f_("parse error at line {}: {}", jerror.line, jerror.text));

        return json<BACKEND>{};
    }

    json<BACKEND> result;
    NATIVE(result) = j;

    return result;
}

template <>
json<BACKEND>
json<BACKEND>::parse (string_view source, error * perr)
{
    return parse(source.data(), source.size(), perr);
}

template <>
json<BACKEND>
json<BACKEND>::parse (std::string const & source, error * perr)
{
    return parse(source.data(), source.size(), perr);
}

template <>
json<BACKEND>
json<BACKEND>::parse (pfs::filesystem::path const & path, error * perr)
{
    json_error_t jerror;

    auto j = json_load_file(pfs::utf8_encode_path(path).c_str()
        ,     JSON_DECODE_ANY
            | JSON_REJECT_DUPLICATES
            | JSON_ALLOW_NUL
        , & jerror);

    if (!j) {
        pfs::throw_or(perr, make_error_code(pfs::errc::backend_error)
            , tr::f_("parse error at {}:{}: {}"
                , jerror.line
                , pfs::utf8_encode_path(path)
                , jerror.text));

        return json<BACKEND>{};
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
        throw error {make_error_code(std::errc::invalid_argument), tr::_("attempt to insert unitialized value")};

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    if (!json_is_object(INATIVE(*self)))
        throw error {make_error_code(errc::incopatible_type), tr::_("object expected")};

    auto copy = json_deep_copy(NATIVE(value));

    if (!copy)
        throw error {make_error_code(pfs::errc::backend_error), tr::_("deep copy failure")};

    auto rc = json_object_setn_new_nocheck(INATIVE(*self)
        , key.data()
        , key.size()
        , copy);

    if (rc != 0)
        throw error {make_error_code(pfs::errc::backend_error), tr::_("object insertion failure")};
}

template void modifiers_interface<JSON, BACKEND>::insert_helper (string_view const &, value_type const &);
template void modifiers_interface<JSON_REF, BACKEND>::insert_helper (string_view const &, value_type const &);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::insert (key_type const & key, value_type && value)
{
    auto self = static_cast<Derived *>(this);

    if (!value)
        throw error {make_error_code(std::errc::invalid_argument), tr::_("attempt to insert unitialized value")};

    if (!INATIVE(*self))
        INATIVE(*self) = json_object();

    if (!json_is_object(INATIVE(*self)))
        throw error {make_error_code(errc::incopatible_type), tr::_("object expected")};

    auto rc = json_object_setn_new_nocheck(INATIVE(*self)
        , key.c_str()
        , key.size()
        , NATIVE(value));

    if (rc != 0)
        throw error {make_error_code(pfs::errc::backend_error), tr::_("object insertion failure")};

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
        INATIVE(*self) = json_array();

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
        INATIVE(*self) = json_array();

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
        INATIVE(*self) = json_array();

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
        INATIVE(*self) = json_array();

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
        throw error {make_error_code(std::errc::invalid_argument), tr::_("attempt to add unitialized value")};

    if (! INATIVE(*self))
        INATIVE(*self) = json_array();

    if (!json_is_array(INATIVE(*self)))
        throw error {make_error_code(errc::incopatible_type), tr::_("array expected")};

    auto copy = json_deep_copy(NATIVE(value));

    if (!copy)
        throw error {make_error_code(pfs::errc::backend_error), tr::_("deep copy failure")};

    auto rc = json_array_append_new(INATIVE(*self), copy);

    if (rc != 0)
        throw error {make_error_code(pfs::errc::backend_error), tr::_("array append failure")};
}

template void modifiers_interface<JSON, BACKEND>::push_back_helper (value_type const &);
template void modifiers_interface<JSON_REF, BACKEND>::push_back_helper (value_type const &);

template <typename Derived, typename Backend>
void
modifiers_interface<Derived, Backend>::push_back (value_type && value)
{
    auto self = static_cast<Derived *>(this);

    if (!value)
        throw error {make_error_code(std::errc::invalid_argument), tr::_("attempt to add unitialized value")};

    if (! INATIVE(*self))
        INATIVE(*self) = json_array();

    if (!json_is_array(INATIVE(*self)))
        throw error {make_error_code(errc::incopatible_type), tr::_("array expected")};

    auto rc = json_array_append_new(INATIVE(*self), NATIVE(value));

    if (rc != 0)
        throw error {make_error_code(pfs::errc::backend_error), tr::_("array append failure")};

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
            throw error {make_error_code(pfs::errc::backend_error), tr::_("stringification failure")};
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
        throw error {make_error_code(errc::incopatible_type), tr::_("array expected")};

    auto ptr = json_array_get(CINATIVE(*self), pos);

    if (!ptr) {
        throw error {
              make_error_code(std::errc::invalid_argument)
            , tr::f_("index is out of bounds: {}", pos)
        };
    }

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
        throw error {make_error_code(errc::incopatible_type), tr::_("object expected")};

    auto ptr = json_object_getn(CINATIVE(*self), key.data(), key.length());

    if (!ptr)
        throw error {
              make_error_code(std::errc::invalid_argument)
            , tr::f_("bad key: {}", pfs::to_string(key))
        };

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
algorithm_interface<Derived, Backend>::for_each (std::function<void (reference)> f) const noexcept
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

template void algorithm_interface<JSON, BACKEND>::for_each (std::function<void (reference)> f) const noexcept;
template void algorithm_interface<JSON_REF, BACKEND>::for_each (std::function<void (reference)> f) const noexcept;

////////////////////////////////////////////////////////////////////////////////
// Iterator interface
////////////////////////////////////////////////////////////////////////////////
template <typename Derived, typename Backend>
typename iterator_interface<Derived, Backend>::iterator
iterator_interface<Derived, Backend>::begin () noexcept
{
    auto self = static_cast<Derived *>(this);

    PFS__TERMINATE(INATIVE(*self), "iterator_interface::begin(): null pointer");

    iterator it;
    it._parent = INATIVE(*self);
    it._index = 0;

    if (json_is_object(INATIVE(*self)))
        it._iter = json_object_iter(it._parent);

    return it;
}

template <typename Derived, typename Backend>
typename iterator_interface<Derived, Backend>::const_iterator
iterator_interface<Derived, Backend>::begin () const noexcept
{
    auto self = static_cast<Derived const *>(this);

    PFS__TERMINATE(CINATIVE(*self), "iterator_interface::begin(): null pointer");

    const_iterator it;
    it._parent = CINATIVE(*self);
    it._index = 0;

    if (json_is_object(CINATIVE(*self)))
        it._iter = json_object_iter(it._parent);

    return it;
}

template iterator_interface<JSON, BACKEND>::iterator iterator_interface<JSON, BACKEND>::begin () noexcept;
template iterator_interface<JSON, BACKEND>::const_iterator iterator_interface<JSON, BACKEND>::begin () const noexcept;
template iterator_interface<JSON_REF, BACKEND>::iterator iterator_interface<JSON_REF, BACKEND>::begin () noexcept;
template iterator_interface<JSON_REF, BACKEND>::const_iterator iterator_interface<JSON_REF, BACKEND>::begin () const noexcept;

template <typename Derived, typename Backend>
typename iterator_interface<Derived, Backend>::iterator
iterator_interface<Derived, Backend>::end () noexcept
{
    iterator it;
    auto self = static_cast<Derived *>(this);

    PFS__TERMINATE(INATIVE(*self), "iterator_interface::end(): null pointer");

    it._parent = INATIVE(*self);

    if (json_is_object(INATIVE(*self))) {
        it._iter = nullptr;
    } else if (json_is_array(INATIVE(*self))) {
        it._index = json_array_size(INATIVE(*self));
    } else {
        it._index = 1;
    }

    return it;
}

template <typename Derived, typename Backend>
typename iterator_interface<Derived, Backend>::const_iterator
iterator_interface<Derived, Backend>::end () const noexcept
{
    const_iterator it;
    auto self = static_cast<Derived const *>(this);

    PFS__TERMINATE(CINATIVE(*self), "iterator_interface::end(): null pointer");

    it._parent = CINATIVE(*self);

    if (json_is_object(CINATIVE(*self))) {
        it._iter = nullptr;
    } else if (json_is_array(CINATIVE(*self))) {
        it._index = json_array_size(CINATIVE(*self));
    } else {
        it._index = 1;
    }

    return it;
}

template iterator_interface<JSON, BACKEND>::iterator iterator_interface<JSON, BACKEND>::end () noexcept;
template iterator_interface<JSON, BACKEND>::const_iterator iterator_interface<JSON, BACKEND>::end () const noexcept;
template iterator_interface<JSON_REF, BACKEND>::iterator iterator_interface<JSON_REF, BACKEND>::end () noexcept;
template iterator_interface<JSON_REF, BACKEND>::const_iterator iterator_interface<JSON_REF, BACKEND>::end () const noexcept;

template <typename ValueType, typename RefType, typename Backend>
template <typename U, typename V>
basic_iterator<ValueType, RefType, Backend>::basic_iterator (basic_iterator<U, V, Backend> other)
{
    this->_parent = other._parent;
    this->_index = other._index;
    this->_iter = other._iter;
}

template basic_iterator<JSON const, JSON_REF const, BACKEND>::basic_iterator (basic_iterator<JSON, JSON_REF, BACKEND>);

template <typename ValueType, typename RefType, typename Backend>
bool basic_iterator<ValueType, RefType, Backend>::equals (basic_iterator<ValueType, RefType, Backend> const & other) const
{
    return this->_parent == other._parent
        && this->_index == other._index
        && this->_iter == other._iter;
}

template bool basic_iterator<JSON, JSON_REF, BACKEND>::equals (basic_iterator<JSON, JSON_REF, BACKEND> const & ) const;
template bool basic_iterator<JSON const, JSON_REF const, BACKEND>::equals (basic_iterator<JSON const, JSON_REF const, BACKEND> const & ) const;

template <typename ValueType, typename RefType, typename Backend>
typename basic_iterator<ValueType, RefType, Backend>::reference
basic_iterator<ValueType, RefType, Backend>::ref ()
{
    if (json_is_object(this->_parent)) {
        if (this->_iter == nullptr)
            throw error {make_error_code(std::errc::result_out_of_range)};

        json_t * ptr = json_object_iter_value(this->_iter);
        char const * key = json_object_iter_key(this->_iter);
        auto key_length = json_object_iter_key_len(this->_iter);

        return reference{BACKEND::ref{ptr, this->_parent, BACKEND::key_type(key, key_length)}};
    } else if (json_is_array(this->_parent)) {
        auto ptr = json_array_get(this->_parent, this->_index);

        if (!ptr)
            throw error {make_error_code(std::errc::result_out_of_range)};

        return reference{BACKEND::ref{ptr, this->_parent, this->_index}};
    }/* else { */
        if (this->_index > 0)
            throw error {make_error_code(std::errc::result_out_of_range)};

        return reference(typename Backend::ref{this->_parent, nullptr, BACKEND::size_type{0}});
    /* } */
}

template basic_iterator<JSON, JSON_REF, BACKEND>::reference basic_iterator<JSON, JSON_REF, BACKEND>::ref ();
template basic_iterator<JSON const, JSON_REF const, BACKEND>::reference basic_iterator<JSON const, JSON_REF const, BACKEND>::ref ();

template <typename ValueType, typename RefType, typename Backend>
void basic_iterator<ValueType, RefType, Backend>::increment (difference_type)
{
    if (json_is_object(this->_parent)) {
        if (this->_iter == nullptr)
            throw error {make_error_code(std::errc::result_out_of_range)};

        this->_iter = json_object_iter_next(this->_parent, this->_iter);
    } else if (json_is_array(this->_parent)) {
        if (this->_index > json_array_size(this->_parent))
            throw error {make_error_code(std::errc::result_out_of_range)};

        ++this->_index;
    } else {
        if (this->_index == 0)
            this->_index = 1;
        else
            throw error {make_error_code(std::errc::result_out_of_range)};
    }
}

template void basic_iterator<JSON, JSON_REF, BACKEND>::increment (difference_type);
template void basic_iterator<JSON const, JSON_REF const, BACKEND>::increment (difference_type);

template <typename ValueType, typename RefType, typename Backend>
void basic_iterator<ValueType, RefType, Backend>::decrement (difference_type)
{
    if (json_is_object(this->_parent)) {
            throw error {make_error_code(errc::incopatible_type)};
    } else if (json_is_array(this->_parent)) {
        if (this->_index == 0)
            throw error {make_error_code(std::errc::result_out_of_range)};

        --this->_index;
    } else {
        if (this->_index == 1)
            this->_index = 0;
        else
            throw error {make_error_code(std::errc::result_out_of_range)};
    }
}

template void basic_iterator<JSON, JSON_REF, BACKEND>::decrement (difference_type);
template void basic_iterator<JSON const, JSON_REF const, BACKEND>::decrement (difference_type);

template <typename ValueType, typename RefType, typename Backend>
bool basic_iterator<ValueType, RefType, Backend>::decrement_support () const
{
    if (json_is_object(this->_parent))
        return false;

    return true;
}

template bool basic_iterator<JSON, JSON_REF, BACKEND>::decrement_support () const;
template bool basic_iterator<JSON const, JSON_REF const, BACKEND>::decrement_support () const;

template <typename ValueType, typename RefType, typename Backend>
typename basic_iterator<ValueType, RefType, Backend>::key_type
basic_iterator<ValueType, RefType, Backend>::key () const
{
    if (!json_is_object(this->_parent))
        throw error {make_error_code(errc::incopatible_type)};

    if (this->_iter == nullptr)
        throw error {make_error_code(std::errc::result_out_of_range)};

    char const * key = json_object_iter_key(this->_iter);

    return BACKEND::key_type(key);
}

template basic_iterator<JSON, JSON_REF, BACKEND>::key_type basic_iterator<JSON, JSON_REF, BACKEND>::key () const;
template basic_iterator<JSON const, JSON_REF const, BACKEND>::key_type basic_iterator<JSON const, JSON_REF const, BACKEND>::key () const;

} // namespace jeyson
