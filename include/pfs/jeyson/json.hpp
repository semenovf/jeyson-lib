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
#include <cstddef>
#include <cstdint>

namespace jeyson {

template <typename Backend>
class json
{
private:
    typename Backend::native_type _d;

public:
    /**
     * Construct empty/invalid JSON value.
     */
    json ();

    /**
     * Construct @c null value.
     */
    explicit json (std::nullptr_t);

    /**
     * Construct boolean value.
     */
    explicit json (bool value);

    /**
     * Construct integer value.
     */
    explicit json (std::intmax_t value);

    /**
     * Construct integer value.
     *
     * @note Convenient constructor to avoid ambiguity.
     */
    explicit json (int value) : json(std::intmax_t{value}) {}

    /**
     * Construct real value from @c double.
     */
    explicit json (double value);

    /**
     * Construct real value from @c float.
     */
    explicit json (float value) : json(double{value}) {}

    /**
     * Construct string value.
     */
    explicit json (std::string const & value);

    /**
     * Construct string value from C-like string.
     */
    explicit json (char const * value);

    /**
     * Construct string value from character sequence with length @a n.
     */
    explicit json (char const * value, std::size_t n);

    /**
     * Copy constructor
     */
    json (json const & other);

    /**
     * Move constructor
     */
    json (json && other);

    ~json ();

    /**
     * Copy assignment
     */
    json & operator = (json const & other);

    /**
     * Move assignment
     */
    json & operator = (json && other);

    /**
     * Check if JSON value is initialized.
     */
    operator bool () const noexcept;

    /**
     * Check if JSON value is null.
     */
    bool is_null () const noexcept;

    /**
     * Check if JSON value is boolean.
     */
    bool is_bool () const noexcept;

    /**
     * Check if JSON value is integer.
     */
    bool is_integer () const noexcept;

    /**
     * Check if JSON value is real.
     */
    bool is_real () const noexcept;

    /**
     * Check if JSON value is string.
     */
    bool is_string () const noexcept;

    /**
     * Check if JSON value is array.
     */
    bool is_array () const noexcept;

    /**
     * Check if JSON value is object.
     */
    bool is_object () const noexcept;
};

} // namespace jeyson
