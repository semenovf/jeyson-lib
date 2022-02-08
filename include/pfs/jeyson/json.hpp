////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "pfs/jeyson/error.hpp"
#include <limits>
#include <string>
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace jeyson {

template <typename Backend>
class json
{
public:
    using value_type      = json;
    using size_type       = typename Backend::size_type;
    using string_type     = typename Backend::string_type;
    using key_type        = typename Backend::key_type;
    using reference       = json;
    using const_reference = const json;

private:
    typename Backend::rep_type _d;

private:
    json (typename Backend::rep_type const & rep);
    json (typename Backend::rep_type && rep);

public:
////////////////////////////////////////////////////////////////////////////////
// Constructors, destructors, assignment operators
////////////////////////////////////////////////////////////////////////////////
    json ();

    /// Construct @c null value.
    explicit json (std::nullptr_t);

    /// Construct boolean value.
    explicit json (bool value);

    /// Construct integer value.
    explicit json (std::intmax_t value);

    /// Construct from any integral type (bool, char, int, etc).
    template <typename T>
    explicit json (T x, typename std::enable_if<std::is_integral<T>::value>::type * = 0)
        : json(static_cast<std::intmax_t>(x))
    {}

    /// Construct real value from @c double.
    explicit json (double value);

    /// Construct from any floating point type.
    template <typename T>
    explicit json (T x, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0)
        : json(static_cast<double>(x))
    {}

    /// Construct string value.
    explicit json (string_type const & value);

    /// Construct string value from C-like string.
    explicit json (char const * value);

    /**
     * Construct string value from character sequence with length @a n.
     * Value may contain null characters or not be null terminated.
     */
    explicit json (char const * value, std::size_t n);

    json (json const & other);

    json (json && other);

    ~json ();

    json & operator = (json const & other);

    json & operator = (json && other);

    /// Check if JSON value is initialized.
    operator bool () const noexcept;

////////////////////////////////////////////////////////////////////////////////
// Capacity
////////////////////////////////////////////////////////////////////////////////
    /**
     * Returns the number of elements in the container (if it is an array
     * or object), @c 1 if value is scalar and @c 0 if value is uninitialized
     * or container is empty.
     */
    size_type size () const noexcept;

    /**
     * Checks when the size is equal to zero.
     */
    bool empty () const noexcept
    {
        return size() == 0;
    }

////////////////////////////////////////////////////////////////////////////////
// Modifiers
////////////////////////////////////////////////////////////////////////////////
    /**
     * Appends the given element @a value to the end of the array.
     * The new element is initialized as a deep copy of @a value.
     *
     * @throws @c error { @c errc::incopatible_type } if @c this is initialized
     *         and it is not an error.
     * @throws @c error { @c errc::invalid_argument } if @a value is uninitialized.
     */
    void push_back (json const & value);

    /**
     * Appends the given element @a value to the end of the array.
     * @a value is moved into the new element.
     *
     * @throws @c error { @c errc::incopatible_type } if @c this is initialized
     *         and it is not an error.
     * @throws @c error { @c errc::invalid_argument } if @a value is uninitialized.
     */
    void push_back (json && value);

////////////////////////////////////////////////////////////////////////////////
// Comparison operators
////////////////////////////////////////////////////////////////////////////////
    template <typename U>
    friend bool operator == (json<U> const & lhs, json<U> const & rhs);

////////////////////////////////////////////////////////////////////////////////
// Type quieries
////////////////////////////////////////////////////////////////////////////////

    /// Check if JSON value is null.
    template <typename U>
    friend bool is_null (json<U> const & j);

    /// Check if JSON value is boolean.
    template <typename U>
    friend bool is_bool (json<U> const & j);

    /// Check if JSON value is integer.
    template <typename U>
    friend bool is_integer (json<U> const & j);

    /// Check if JSON value is real.
    template <typename U>
    friend bool is_real (json<U> const & j);

    /// Check if JSON value is string.
    template <typename U>
    friend bool is_string (json<U> const & j);

    /// Check if JSON value is array.
    template <typename U>
    friend bool is_array (json<U> const & j);

    /// Check if JSON value is object.
    template <typename U>
    friend bool is_object (json<U> const & j);
};

template <typename Backend>
bool operator == (json<Backend> const & lhs, json<Backend> const & rhs);

template <typename Backend>
inline bool operator != (json<Backend> const & lhs, json<Backend> const & rhs)
{
    return !(lhs == rhs);
}

template <typename Backend>
bool is_null (json<Backend> const & j);

template <typename Backend>
bool is_bool (json<Backend> const & j);

template <typename Backend>
bool is_integer (json<Backend> const & j);

template <typename Backend>
bool is_real (json<Backend> const & j);

template <typename Backend>
bool is_string (json<Backend> const & j);

template <typename Backend>
bool is_array (json<Backend> const & j);

template <typename Backend>
bool is_object (json<Backend> const & j);

template <typename Backend>
inline bool is_scalar (json<Backend> const & j)
{
    return is_null(j) || is_bool(j) || is_integer(j)
        || is_real(j) || is_string(j);
}

template <typename Backend>
inline bool is_structured (json<Backend> const & j)
{
    return is_array(j) || is_object(j);
}

} // namespace jeyson
