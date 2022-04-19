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
#include "pfs/filesystem.hpp"
#include "pfs/optional.hpp"
#include "pfs/string_view.hpp"
#include "pfs/type_traits.hpp"
#include <string>
#include <type_traits>
#include <cstddef>
#include <cstdint>

namespace jeyson {

template <typename Backend>
class json;

template <typename Backend>
class json_ref;

////////////////////////////////////////////////////////////////////////////////
// Traits interface
////////////////////////////////////////////////////////////////////////////////
template <typename Backend>
class traits_interface
{
public:
    //--------------------------------------------------------------------------
    // Type quieries
    //--------------------------------------------------------------------------

    /// Check if JSON value is null.
    bool is_null () const noexcept;

    /// Check if JSON value is boolean.
    bool is_bool () const noexcept;

    /// Check if JSON value is integer.
    bool is_integer () const noexcept;

    /// Check if JSON value is real.
    bool is_real () const noexcept;

    /// Check if JSON value is string.
    bool is_string () const noexcept;

    /// Check if JSON value is array.
    bool is_array () const noexcept;

    /// Check if JSON value is an object.
    bool is_object () const noexcept;

    /// Check if JSON value is scalar (neither an array nor an object) value.
    bool is_scalar () const noexcept
    {
        return is_null() || is_bool() || is_integer()
            || is_real() || is_string();
    }

    /// Check if JSON value is sctuctured (array or object) value.
    bool is_structured () const noexcept
    {
        return is_array() || is_object();
    }
};

////////////////////////////////////////////////////////////////////////////////
// Modifiers interface
////////////////////////////////////////////////////////////////////////////////
template <typename Backend>
class modifiers_interface
{
public:
    using size_type = typename Backend::size_type;
    using key_type = typename Backend::key_type;

public:
    /**
     * Inserts copy of @a value into the object overriding if the object
     * already contains an element with an equivalent key.
     *
     * @param key Key by which the @a value is inserted.
     * @param value Value to be inserted.
     *
     * @return @c true if insertion took place, @c false otherwise.
     *
     * @throw @c error { @c errc::invalid_argument } if @a value is uninitialized.
     * @throw @c error { @c errc::incopatible_type } if @c this is initialized
     *        and it is not an object.
     * @throw @c error { @c errc::backend_error } if backend call(s) results a failure.
     */
    void insert (key_type const & key, json<Backend> const & value);

    /**
     * Inserts @a value into the object overriding if the object already contains
     * an element with an equivalent key.
     */
    void insert (key_type const & key, json<Backend> && value);

    /**
     * Appends the given element @a value to the end of the array.
     * The new element is initialized as a deep copy of @a value.
     *
     * @throw @c error { @c errc::invalid_argument } if @a value is uninitialized.
     * @throw @c error { @c errc::incopatible_type } if @c this is initialized
     *        and it is not an error.
     * @throw @c error { @c errc::backend_error } if backend call(s) results a failure.
     */
    void push_back (json<Backend> const & value);

    /**
     * Appends the given element @a value to the end of the array.
     * @a value is moved into the new element.
     *
     * @throws @c error { @c errc::invalid_argument } if @a value is uninitialized.
     * @throws @c error { @c errc::incopatible_type } if @c this is initialized
     *         and it is not an error.
     * @throw @c error { @c errc::backend_error } if backend call(s) results a failure.
     */
    void push_back (json<Backend> && value);
};

////////////////////////////////////////////////////////////////////////////////
// Capacity interface
////////////////////////////////////////////////////////////////////////////////
template <typename Backend>
class capacity_interface
{
public:
    using size_type = typename Backend::size_type;

public:
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
};

////////////////////////////////////////////////////////////////////////////////
// Converter interface
////////////////////////////////////////////////////////////////////////////////
template <typename Backend>
class converter_interface
{
public:
    //--------------------------------------------------------------------------
    // Stringification
    //--------------------------------------------------------------------------
    std::string to_string () const;
};

////////////////////////////////////////////////////////////////////////////////
// Element access interface
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename U = void>
struct lexical_cast
{
    T operator () () const noexcept; // default value when casting is unavailable
    T operator () (std::nullptr_t, bool * success) const noexcept;
    T operator () (bool, bool * success) const noexcept;
    T operator () (std::intmax_t, bool * success) const noexcept;
    T operator () (double, bool * success) const noexcept;
    T operator () (pfs::string_view const &, bool * success) const noexcept;
    T operator () (std::size_t /*container size*/, bool /*is_array*/, bool * success) const noexcept;

    T operator () (char const * s,  bool * success) const noexcept
    {
        return this->operator() (pfs::string_view(s, std::strlen(s)), success);
    }
};

template <typename T>
struct lexical_cast<T, typename
        std::enable_if<
            std::is_integral<pfs::remove_cvref_t<T>>::value
            && std::is_signed<T>::value
            && !std::is_same<bool, pfs::remove_cvref_t<T>>::value
            && !std::is_same<std::intmax_t, pfs::remove_cvref_t<T>>::value>::type>
{
    T operator () () const noexcept
    {
        return lexical_cast<std::intmax_t>{}.operator() ();
    }

    T operator () (std::nullptr_t, bool * success) const noexcept
    {
        return lexical_cast<std::intmax_t>{}.operator() (nullptr, success);
    }

    T operator () (bool v, bool * success) const noexcept
    {
        return lexical_cast<std::intmax_t>{}.operator() (v, success);
    }

    T operator () (std::intmax_t v, bool * success) const noexcept
    {
        // Overflow or underflow
        if (v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max()) {
            *success = false;
            return this->operator() ();
        }

        return lexical_cast<std::intmax_t>{}.operator() (v, success);
    }

    T operator () (double v, bool * success) const noexcept
    {
        auto x = lexical_cast<std::intmax_t>{}.operator() (v, success);
        return this->operator() (x, success);
    }

    T operator () (pfs::string_view const & v, bool * success) const noexcept
    {
        auto x = lexical_cast<std::intmax_t>{}.operator() (v, success);
        return this->operator() (x, success);
    }

    T operator () (std::size_t size, bool is_array, bool * success) const noexcept
    {
        auto x = lexical_cast<std::intmax_t>{}.operator() (size, is_array, success);
        return this->operator() (x, success);
    }

    T operator () (char const * s,  bool * success) const noexcept
    {
        auto x = lexical_cast<std::intmax_t>{}.operator() (s, success);
        return this->operator() (x, success);
    }
};

template <typename T>
struct lexical_cast<T, typename
        std::enable_if<
            std::is_integral<pfs::remove_cvref_t<T>>::value
            && std::is_unsigned<T>::value
            && !std::is_same<bool, pfs::remove_cvref_t<T>>::value
            && !std::is_same<std::intmax_t, pfs::remove_cvref_t<T>>::value>::type>
{
    T operator () () const noexcept
    {
        return lexical_cast<std::intmax_t>{}.operator() ();
    }

    T operator () (std::nullptr_t, bool * success) const noexcept
    {
        return lexical_cast<std::intmax_t>{}.operator() (nullptr, success);
    }

    T operator () (bool v, bool * success) const noexcept
    {
        return lexical_cast<std::intmax_t>{}.operator() (v, success);
    }

    T operator () (std::intmax_t v, bool * success) const noexcept
    {
        // Overflow
        if (static_cast<std::uintmax_t>(v) > std::numeric_limits<T>::max()) {
            *success = false;
            return this->operator() ();
        }

        return lexical_cast<std::intmax_t>{}.operator() (v, success);
    }

    T operator () (double v, bool * success) const noexcept
    {
        auto x = lexical_cast<std::intmax_t>{}.operator() (v, success);
        return this->operator() (x, success);
    }

    T operator () (pfs::string_view const & v, bool * success) const noexcept
    {
        auto x = lexical_cast<std::intmax_t>{}.operator() (v, success);
        return this->operator() (x, success);
    }

    T operator () (std::size_t size, bool is_array, bool * success) const noexcept
    {
        auto x = lexical_cast<std::intmax_t>{}.operator() (size, is_array, success);
        return this->operator() (x, success);
    }

    T operator () (char const * s,  bool * success) const noexcept
    {
        auto x = lexical_cast<std::intmax_t>{}.operator() (s, success);
        return this->operator() (x, success);
    }
};

template <typename T>
struct lexical_cast<T, typename
        std::enable_if<
            std::is_floating_point<pfs::remove_cvref_t<T>>::value
            && !std::is_same<double, pfs::remove_cvref_t<T>>::value>::type>
{
    T operator () () const noexcept
    {
        return lexical_cast<double>{}.operator() ();
    }

    T operator () (std::nullptr_t, bool * success) const noexcept
    {
        return lexical_cast<double>{}.operator() (nullptr, success);
    }

    T operator () (bool v, bool * success) const noexcept
    {
        return lexical_cast<double>{}.operator() (v, success);
    }

    T operator () (std::intmax_t v, bool * success) const noexcept
    {
        return lexical_cast<double>{}.operator() (v, success);
    }

    T operator () (double v, bool * success) const noexcept
    {
        if (v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max()) {
            *success = false;
            return this->operator() ();
        }

        return lexical_cast<double>{}.operator() (v, success);
    }

    T operator () (pfs::string_view const & v, bool * success) const noexcept
    {
        auto x = lexical_cast<double>{}.operator() (v, success);
        return this->operator() (x, success);
    }

    T operator () (std::size_t size, bool is_array, bool * success) const noexcept
    {
        auto x = lexical_cast<double>{}.operator() (size, is_array, success);
        return this->operator() (x, success);
    }

    T operator () (char const * s,  bool * success) const noexcept
    {
        auto x = lexical_cast<double>{}.operator() (s, success);
        return this->operator() (x, success);
    }
};

template <typename Backend>
class element_accessor_interface
{
public:
    using size_type = typename Backend::size_type;
    using key_type = typename Backend::key_type;
    using reference = json_ref<Backend>;
    using const_reference = json_ref<Backend> const;

private:
    bool bool_value () const noexcept;
    std::intmax_t integer_value () const noexcept;
    double real_value () const noexcept;
    pfs::string_view string_value () const noexcept;
    std::size_t array_size () const noexcept;
    std::size_t object_size () const noexcept;

public:
    /**
     * Returns a reference to the element at specified location @a pos.
     * In case of out of bounds, the result is a reference to an invalid value.
     */
    reference operator [] (size_type pos) noexcept;

    /**
     * Returns a constant reference to the element at specified location @a pos.
     * In case of out of bounds, the result is a reference to an invalid value.
     */
    const_reference operator [] (size_type pos) const noexcept;

    /**
     * Returns a reference to the element at specified location @a pos.
     *
     * @note This overloaded methods needs to avoid ambiguity for zero (0) index.
     */
    template <typename IndexT = int>
    typename std::enable_if<std::is_integral<IndexT>::value
        && !std::is_same<size_type, IndexT>::value, reference>::type
    operator [] (IndexT pos) noexcept
    {
        return this->operator[] (static_cast<size_type>(pos));
    }

    /**
     * Returns a constant reference to the element at specified location @a pos.
     *
     * @note This overloaded methods needs to avoid ambiguity for zero (0) index
     */
    template <typename IndexT = int>
    typename std::enable_if<std::is_integral<IndexT>::value
        && !std::is_same<size_type, IndexT>::value, const_reference>::type
    operator [] (IndexT pos) const noexcept
    {
        return this->operator[] (static_cast<size_type>(pos));
    }

    /**
     * Returns a reference to the value that is mapped to a key equivalent
     * to @a key, performing an insertion if such key does not already exist.
     */
    reference operator [] (pfs::string_view key) noexcept;
    reference operator [] (key_type const & key) noexcept;
    reference operator [] (char const * key) noexcept;

    /**
     * Returns a reference to the value that is mapped to a key equivalent
     * to @a key. In case of out of range, the result is a reference to an
     * invalid value.
     */
    const_reference operator [] (pfs::string_view key) const noexcept;
    const_reference operator [] (key_type const & key) const noexcept;
    const_reference operator [] (char const * key) const noexcept;

    /**
     * Returns a reference to the element at specified location @a pos.
     *
     * @throw @c error { @c errc::incopatible_type } if @c this is uninitialized
     *        or it is not an array.
     * @throw @c error { @c errc::out_of_range } if @a pos is out of bounds.
     */
    reference at (size_type pos) const;

    /**
     * Returns a reference to the element at specified location @a pos.
     *
     * @note This overloaded methods needs to avoid ambiguity for zero (0) index
     */
    template <typename IndexT = int>
    typename std::enable_if<std::is_integral<IndexT>::value
        && !std::is_same<size_type, IndexT>::value, reference>::type
    at (IndexT pos) const
    {
        return this->at(static_cast<size_type>(pos));
    }

    /**
     * Returns a reference to the value that is mapped to a key equivalent
     * to @a key.
     *
     * @throw @c error { @c errc::incopatible_type } if @c this is uninitialized
     *        or it is not an object.
     * @throw @c error { @c errc::out_of_range } if an element by @a key not found.
     */
    reference at (pfs::string_view key) const;
    reference at (key_type const & key) const;
    reference at (char const * key) const;

    /**
     * Returns the value stored in JSON value/reference.
     *
     * @param success Reference to store the result of convertion JSON
     *        value/reference to specified type.
     */
    template <typename T>
    T get (bool & success) const noexcept
    {
        lexical_cast<T> cast;
        auto self = reinterpret_cast<traits_interface<Backend> const *>(this);
        success = true;

        if (self->is_bool()) {
            return cast(bool_value(), & success);
        } else if (self->is_integer()) {
            return cast(integer_value(), & success);
        } else if (self->is_real()) {
            return cast(real_value(), & success);
        } else if (self->is_string()) {
            return cast(string_value(), & success);
        } else if (self->is_array()) {
            return cast(array_size(), true, & success);
        } else if (self->is_object()) {
            return cast(object_size(), true, & success);
        } else if (self->is_null()) {
            return cast(nullptr, & success);
        } else {
            success = false;
        }

        return cast();
    }

    /**
     * Returns the value stored in JSON value/reference.
     *
     * @throw @c error { @c errc::incopatible_type } if JSON value/reference
     *        stored incopatible to @a T type.
     */
    template <typename T>
    T get () const
    {
        bool success = true;
        auto result = get<T>(success);

        if (!success)
            JEYSON__THROW(error{errc::incopatible_type});

        return result;
    }

    /**
     * Returns the value stored in JSON value/reference.
     *
     * @throw @c error { @c errc::incopatible_type } if JSON value/reference
     *        stored incopatible to @a T type.
     */
    template <typename T>
    T get_or (T const & alt) const noexcept
    {
        bool success = true;
        auto result = get<T>(success);
        return success ? result : alt;
    }
};

////////////////////////////////////////////////////////////////////////////////
// JSON reference
////////////////////////////////////////////////////////////////////////////////
template <typename Backend>
class json_ref: public Backend::ref
    , public traits_interface<Backend>
    , public modifiers_interface<Backend>
    , public capacity_interface<Backend>
    , public converter_interface<Backend>
    , public element_accessor_interface<Backend>
{
    friend class json<Backend>;
    friend class element_accessor_interface<Backend>;

public:
    using ref_type  = typename Backend::ref;
    using size_type = typename Backend::size_type;
    using key_type  = typename Backend::key_type;
    using reference       = json_ref<Backend>;
    using const_reference = json_ref<Backend> const;

private:
    json_ref ();

public:
    json_ref (typename Backend::ref &&);
    json_ref (json_ref &&);
    ~json_ref ();

    /// Check if JSON reference is valid.
    operator bool () const noexcept;
};

////////////////////////////////////////////////////////////////////////////////
// JSON value
////////////////////////////////////////////////////////////////////////////////
template <typename Backend>
class json: public Backend::rep
    , public traits_interface<Backend>
    , public modifiers_interface<Backend>
    , public capacity_interface<Backend>
    , public converter_interface<Backend>
    , public element_accessor_interface<Backend>
{
public:
    using value_type      = json;
    using size_type       = typename Backend::size_type;
    using string_type     = typename Backend::string_type;
    using key_type        = typename Backend::key_type;
    using reference       = json_ref<Backend>;
    using const_reference = json_ref<Backend> const;

private:
    using rep_type = typename Backend::rep;

public:
    /// Check if JSON value is initialized.
    operator bool () const noexcept;

    //--------------------------------------------------------------------------
    // Constructors, destructors, assignment operators
    //--------------------------------------------------------------------------
    json ();

    /// Construct @c null value.
    json (std::nullptr_t);

    /// Construct boolean value.
    json (bool value);

    /// Construct integer value.
    json (std::intmax_t value);

    /// Construct from any integral type (bool, char, int, etc).
    template <typename T>
    json (T x, typename std::enable_if<std::is_integral<T>::value>::type * = 0)
        : json(static_cast<std::intmax_t>(x))
    {}

    /// Construct real value from @c double.
    json (double value);

    /// Construct from any floating point type.
    template <typename T>
    json (T x, typename std::enable_if<std::is_floating_point<T>::value>::type * = 0)
        : json(static_cast<double>(x))
    {}

    /// Construct string value.
    json (string_type const & value);

    /// Construct string value from C-like string.
    json (char const * value);

    /**
     * Construct string value from character sequence with length @a n.
     * Value may contain null characters or not be null terminated.
     */
    json (char const * value, std::size_t n);

    json (json const & other);

    json (json && other);

    ~json ();

    json & operator = (json const & other);

    json & operator = (json && other);

    //--------------------------------------------------------------------------
    // Modifiers
    //--------------------------------------------------------------------------
    /**
     * Exchanges the contents of the JSON value with those of @a other.
     */
    void swap (json & other);

    //--------------------------------------------------------------------------
    // Save
    //--------------------------------------------------------------------------
    /**
     * Writes the JSON representaion to the file @a path. If @a path already
     * exists, it is overwritten.
     *
     * @param path Path to the file to save data.
     * @param compact Save in compact representation.
     * @param indent Number of spaces for indentation (ignored if @a compact
     *        is @c true).
     * @param precision The precision for real numbers output.
     *
     * @throw @c error { @c errc::backend_error } if backend call(s) results a failure.
     */
    void save (pfs::filesystem::path const & path
        , bool compact = false
        , int indent = 4
        , int precision = 17);

    //--------------------------------------------------------------------------
    // Comparison operators
    //--------------------------------------------------------------------------
    template <typename U>
    friend bool operator == (json<U> const & lhs, json<U> const & rhs);

    //--------------------------------------------------------------------------
    // Parsing
    //--------------------------------------------------------------------------
    /**
     * Decodes JSON from string
     */
    static json parse (std::string const & source);

    /**
     * Decodes JSON from file
     */
    static json parse (pfs::filesystem::path const & path);
};

template <typename Backend>
bool operator == (json<Backend> const & lhs, json<Backend> const & rhs);

template <typename Backend>
inline bool operator != (json<Backend> const & lhs, json<Backend> const & rhs)
{
    return !(lhs == rhs);
}

template <typename Backend>
inline bool is_null (json<Backend> const & j) noexcept
{
    return j.is_null();
}

template <typename Backend>
inline bool is_null (json_ref<Backend> const & j) noexcept
{
    return j.is_null();
}

template <typename Backend>
inline bool is_bool (json<Backend> const & j) noexcept
{
    return j.is_bool();
}

template <typename Backend>
inline bool is_bool (json_ref<Backend> const & j) noexcept
{
    return j.is_bool();
}

template <typename Backend>
inline bool is_integer (json<Backend> const & j) noexcept
{
    return j.is_integer();
}

template <typename Backend>
inline bool is_integer (json_ref<Backend> const & j) noexcept
{
    return j.is_integer();
}

template <typename Backend>
inline bool is_real (json<Backend> const & j) noexcept
{
    return j.is_real();
}

template <typename Backend>
inline bool is_real (json_ref<Backend> const & j) noexcept
{
    return j.is_real();
}

template <typename Backend>
inline bool is_string (json<Backend> const & j) noexcept
{
    return j.is_string();
}

template <typename Backend>
inline bool is_string (json_ref<Backend> const & j) noexcept
{
    return j.is_string();
}

template <typename Backend>
inline bool is_array (json<Backend> const & j) noexcept
{
    return j.is_array();
}

template <typename Backend>
inline bool is_array (json_ref<Backend> const & j) noexcept
{
    return j.is_array();
}

template <typename Backend>
inline bool is_object (json<Backend> const & j) noexcept
{
    return j.is_object();
}

template <typename Backend>
inline bool is_object (json_ref<Backend> const & j) noexcept
{
    return j.is_object();
}

template <typename Backend>
inline bool is_scalar (json<Backend> const & j) noexcept
{
    return j.is_scalar();
}

template <typename Backend>
inline bool is_scalar (json_ref<Backend> const & j) noexcept
{
    return j.is_scalar();
}

template <typename Backend>
inline bool is_structured (json<Backend> const & j) noexcept
{
    return j.is_structured();
}

template <typename Backend>
inline bool is_structured (json_ref<Backend> const & j) noexcept
{
    return j.is_structured();
}

template <typename T, typename Backend>
inline T get (json<Backend> const & j, bool & success) noexcept
{
    return j.template get<T>(success);
}

template <typename T, typename Backend>
inline T get (json_ref<Backend> const & j, bool & success) noexcept
{
    return j.template get<T>(success);
}

template <typename T, typename Backend>
inline T get (json<Backend> const & j)
{
    return j.template get<T>();
}

template <typename T, typename Backend>
inline T get (json_ref<Backend> const & j)
{
    return j.template get<T>();
}

template <typename T, typename Backend>
inline T get_or (json<Backend> const & j, T const & alt) noexcept
{
    return j.template get<T>(alt);
}

template <typename T, typename Backend>
inline T get_or (json_ref<Backend> const & j, T const & alt) noexcept
{
    return j.template get<T>(alt);
}

template <typename Backend>
inline void swap (json<Backend> & a, json<Backend> & b)
{
    a.swap(b);
}

template <typename Backend>
inline std::string to_string (json<Backend> const & j)
{
    return j.to_string();
}

template <typename Backend>
inline std::string to_string (json_ref<Backend> const & j)
{
    return j.to_string();
}

} // namespace jeyson
