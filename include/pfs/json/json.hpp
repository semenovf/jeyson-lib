////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-json](https://github.com/semenovf/pfs-json) library.
//
// Changelog:
//      2019.10.05 Initial version
//      2019.12.05 Error-specific code moved into `error.h`
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "constants.hpp"
#include "error.hpp"
#include "iterator.hpp"
#include <list>
#include <map>

namespace pfs {
namespace json {

////////////////////////////////////////////////////////////////////////////////
// value: general JSON class
////////////////////////////////////////////////////////////////////////////////

// Minimal JSON / JSON Value class traits (according to parser requirements)
//
// class json {
// public:
//      using bool_type     = <boolean type>
//      using integer_type  = <integer type>
//      using uinteger_type = <unsigned integer type>
//      using real_type     = <floating point type> // float or double
//      using string_type   = <string type>
//      using array_type    = <array type>
//      using object_type   = <key-value mapping type> // key is of type `string_type`
//
// public:
//      json & operator = ()
//
//      bool is_null () const
//      bool is_boolean () const
//      bool is_number () const
//      bool is_string () const
//      bool is_array () const
//      bool is_object () const
//
//      json & operator = (json &&)
//      json & operator = (bool)
//      json & operator = (<integer type>)
//      json & operator = (<unsigned integer type>)
//      json & operator = (<floating point type>)
//      json & operator = (string_type &&)
//      json & operator = (array_type &&)
//      json & operator = (object_type &&)
// };

template <typename ValueType>
using array_type = std::list<ValueType>;

template <typename KeyType, typename ValueType>
using object_type = std::map<KeyType, ValueType>;

inline std::string to_string (type_enum t)
{
    switch (t) {
        case type_enum::null:     return std::string("null");
        case type_enum::boolean:  return std::string("boolean");
        case type_enum::integer:  return std::string("integer");
        case type_enum::uinteger: return std::string("uinteger");
        case type_enum::real:     return std::string("real");
        case type_enum::string:   return std::string("string");
        case type_enum::array:    return std::string("array");;
        case type_enum::object:   return std::string("object");
        default: break;
    }

    return std::string();
}

template <typename StringType, typename T>
typename std::enable_if<std::is_same<StringType, std::string>::value, StringType>::type
stringify (T const & value)
{
    using std::to_string;
    return to_string(value);
}

template <typename StringType>
StringType stringify (bool value)
{
    return StringType(value ? "true" : "false");
}

// Reference JSON class
template <typename BoolType = bool
        , typename IntegerType = intmax_t
        , typename UIntegerType = uintmax_t
        , typename RealType = double
        , typename StringType = std::string
        , template <typename> class ArrayType = array_type
        , template <typename, typename> class ObjectType = object_type>
class value
{
public:
    using boolean_type    = BoolType;
    using integer_type    = IntegerType;
    using uinteger_type   = UIntegerType;
    using real_type       = RealType;
    using string_type     = StringType;
    using array_type      = ArrayType<value>;
    using object_type     = ObjectType<StringType, value>;
    using size_type       = std::size_t;
    using reference       = value &;
    using const_reference = value const &;

    using iterator = basic_iterator<value>;
    using const_iterator = basic_iterator<const value>;
//     using reverse_iterator = json_reverse_iterator<typename basic_json::iterator>;
//     using const_reverse_iterator = json_reverse_iterator<typename basic_json::const_iterator>;

protected:
    type_enum _type;

    union value_rep {
        boolean_type  boolean_value;
        integer_type  integer_value;
        uinteger_type uinteger_value;
        real_type     real_value;
        string_type * string_value;
        array_type *  array_value;
        object_type * object_value;

        value_rep () = default;
        value_rep (boolean_type v)  noexcept : boolean_value(v) {}
        value_rep (integer_type v)  noexcept : integer_value(v) {}
        value_rep (uinteger_type v) noexcept : uinteger_value(v) {}
        value_rep (real_type v)     noexcept : real_value(v) {}
        value_rep (string_type const & v) : string_value(new string_type(v)) {}
        value_rep (string_type && v) : string_value(new string_type(std::forward<string_type>(v))) {}
        value_rep (array_type const & v) : array_value(new array_type(v)) {}
        value_rep (array_type && v) : array_value(new array_type(std::forward<array_type>(v))) {}
        value_rep (object_type const & v) : object_value(new object_type(v)) {}
        value_rep (object_type && v) : object_value(new object_type(std::forward<object_type>(v))) {}

        value_rep (type_enum t)
        {
            switch (t) {
                case type_enum::object:
                    object_value = new object_type;
                    break;

                case type_enum::array:
                    array_value = new array_type;
                    break;

                case type_enum::string:
                    string_value = new string_type("");
                    break;

                case type_enum::boolean:
                    boolean_value = boolean_type(false);
                    break;

                case type_enum::integer:
                    integer_value = integer_type(0);
                    break;

                case type_enum::uinteger:
                    uinteger_value = uinteger_type(0);
                    break;

                case type_enum::real:
                    real_value = real_type(0.0);
                    break;

                case type_enum::null:
                default:
                    object_value = nullptr;
                    break;
            }
        }

        void destroy (type_enum t)
        {
            switch (t) {
                case type_enum::string:
                    if (string_value) {
                        delete string_value;
                        string_value = nullptr;
                    }
                    break;

                case type_enum::array:
                    if (array_value) {
                        delete array_value;
                        array_value = nullptr;
                    }
                    break;

                case type_enum::object:
                    if (object_value) {
                        delete object_value;
                        object_value = nullptr;
                    }
                    break;

                default:
                    break;
            }
        }
    } _value;

public:
    explicit value (type_enum t) : _type(t), _value(t)
    {}

    explicit value (std::nullptr_t = nullptr) : _type(type_enum::null)
    {}

    explicit value (bool v) : _type(type_enum::boolean), _value(boolean_type{v})
    {}

    template <typename T
            , typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0>
    explicit value (T v) : _type(type_enum::integer), _value(integer_type{v})
    {}

    template <typename T
            , typename std::enable_if<std::is_integral<T>::value && !std::is_signed<T>::value, int>::type = 0>
    explicit value (T v) : _type(type_enum::uinteger), _value(uinteger_type{v})
    {}

    template <typename T
            , typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    explicit value (T v) : _type(type_enum::real), _value(real_type{v})
    {}

    explicit value (string_type const & v)
        : _type(type_enum::string)
        , _value(v)
    {}

    /**
     */
    explicit value (string_type && v)
        : _type(type_enum::string)
        , _value(std::forward<string_type>(v))
    {}

    /**
     * C-string must be converted to @c string_type
     */
    explicit value (char const * v)
        : _type(type_enum::string)
        , _value(string_type(v))
    {}

    /**
     */
    ~value ()
    {
        _value.destroy(_type);
    }

    /**
     */
    type_enum type () const noexcept
    {
        return _type;
    }

    /**
     */
    bool is_null () const noexcept
    {
        return _type == type_enum::null;
    }

    /**
     */
    bool is_boolean () const noexcept
    {
        return _type == type_enum::boolean;
    }

    /**
     */
    bool is_integer () const noexcept
    {
        return _type == type_enum::integer;
    }

    /**
     */
    bool is_uinteger () const noexcept
    {
        return _type == type_enum::uinteger;
    }

    /**
     */
    bool is_real () const noexcept
    {
        return _type == type_enum::real;
    }

    /**
     */
    bool is_number () const noexcept
    {
        return is_integer() || is_uinteger() || is_real();
    }

    /**
     */
    bool is_string () const noexcept
    {
        return _type == type_enum::string;
    }

    /**
     */
    bool is_array () const noexcept
    {
        return _type == type_enum::array;
    }

    /**
     */
    bool is_object () const noexcept
    {
        return _type == type_enum::object;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Cast operations
    ////////////////////////////////////////////////////////////////////////////

    // Casting to boolean
    template <typename T>
    typename std::enable_if<std::is_same<bool, T>::value,T>::type
    get () const
    {
        switch (_type) {
            case type_enum::boolean:
                return _value.boolean_value;

            case type_enum::integer:
            case type_enum::uinteger:
                return _value.integer_value != integer_type(0);

            case type_enum::real:
                return _value.real_value != real_type(0.0);

            default:
                break;
        }

        throw make_cast_exception(to_string(_type), "boolean");
        return T();
    }

    // Casting to signed integers
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value
            && !std::is_unsigned<T>::value
            && !std::is_same<bool, T>::value, T>::type
    get () const
    {
        switch (_type) {
            case type_enum::boolean:  return static_cast<T>(_value.boolean_value);
            case type_enum::integer:  return static_cast<T>(_value.integer_value);
            case type_enum::uinteger: return static_cast<T>(_value.uinteger_value);
            case type_enum::real:     return static_cast<T>(_value.real_value);
            default: break;
        }

        throw make_cast_exception(to_string(_type), "signed integer");
        return T();
    }

    // Casting to unsigned integers
    template <typename T>
    typename std::enable_if<std::is_integral<T>::value
            && std::is_unsigned<T>::value
            && !std::is_same<bool, T>::value, T>::type
    get () const
    {
        switch (_type) {
            case type_enum::boolean:  return static_cast<T>(_value.boolean_value);
            case type_enum::integer:  return static_cast<T>(_value.integer_value);
            case type_enum::uinteger: return static_cast<T>(_value.uinteger_value);
            case type_enum::real:     return static_cast<T>(_value.real_value);
            default: break;
        }

        throw make_cast_exception(to_string(_type), "unsigned integer");
        return T();
    }

    // Casting to floating point
    template <typename T>
    typename std::enable_if<std::is_floating_point<T>::value, T>::type
    get () const
    {
        switch (_type) {
            case type_enum::boolean:  return static_cast<T>(_value.boolean_value);
            case type_enum::integer:  return static_cast<T>(_value.integer_value);
            case type_enum::uinteger: return static_cast<T>(_value.uinteger_value);
            case type_enum::real:     return static_cast<T>(_value.real_value);
            default: break;
        }

        throw make_cast_exception(to_string(_type), "floating point");
        return T();
    }

    // Casting to string
    template <typename T>
    typename std::enable_if<std::is_same<string_type, T>::value, T>::type
    get () const
    {
        using std::to_string;

        switch (_type) {
            case type_enum::boolean:  return stringify<string_type>(_value.boolean_value);
            case type_enum::integer:  return stringify<string_type>(_value.integer_value);
            case type_enum::uinteger: return stringify<string_type>(_value.uinteger_value);
            case type_enum::real:     return stringify<string_type>(_value.real_value);
            case type_enum::string:   return *_value.string_value;
            default: break;
        }

        throw make_cast_exception(to_string(_type), "string");
        return T();
    }

    ////////////////////////////////////////////////////////////////////////////
    // Iterators
    ////////////////////////////////////////////////////////////////////////////
private:
    template <typename Iterator>
    Iterator begin () noexcept
    {
        switch (_type) {
            case type_enum::null:     return Iterator{};
            case type_enum::boolean:  return Iterator{& _value.boolean_value};
            case type_enum::integer:  return Iterator{& _value.integer_value};
            case type_enum::uinteger: return Iterator{& _value.uinteger_value};
            case type_enum::real:     return Iterator{& _value.real_value};
            case type_enum::string:   return Iterator{& _value.string_value};
            case type_enum::array:    return Iterator{_value.array_value.begin()};
            case type_enum::object:   return Iterator{_value.object_value.begin()};
            default:
                static_assert("Unexpected choice");
                break;
        }

        return Iterator{};
    }

    template <typename Iterator>
    Iterator end () noexcept
    {
        switch (_type) {
            case type_enum::null:     return Iterator{};
            case type_enum::boolean:  return Iterator{(& _value.boolean_value) + 1};
            case type_enum::integer:  return Iterator{(& _value.integer_value) + 1};
            case type_enum::uinteger: return Iterator{(& _value.uinteger_value) + 1};
            case type_enum::real:     return Iterator{(& _value.real_value) + 1};
            case type_enum::string:   return Iterator{(& _value.string_value) + 1};
            case type_enum::array:    return Iterator{_value.array_value.end()};
            case type_enum::object:   return Iterator{_value.object_value.end()};
            default:
                static_assert("Unexpected choice");
                break;
        }

        return Iterator{};
    }

public:
    iterator begin () noexcept
    {
        return begin<iterator>();
    }

    const_iterator begin () const noexcept
    {
        return begin<const_iterator>();
    }

    const_iterator cbegin () const noexcept
    {
        return begin<const_iterator>();
    }

    iterator end () noexcept
    {
        return end<iterator>();
    }

    const_iterator end () const noexcept
    {
        return end<const_iterator>();
    }

    const_iterator cend () const noexcept
    {
        return end<const_iterator>();
    }

    // TODO
//     reverse_iterator rbegin () noexcept;
//
//     const_reverse_iterator rbegin () const noexcept;
//
//     const_reverse_iterator crbegin () const noexcept;
//
//     reverse_iterator rend () noexcept;
//
//     const_reverse_iterator rend () const noexcept;
//
//     const_reverse_iterator crend () const noexcept;

    ////////////////////////////////////////////////////////////////////////////
    // Collection specific methods
    //
    // Capacity
    ////////////////////////////////////////////////////////////////////////////
    /**
     * @brief Returns the number of elements in a JSON value.
     * @return The return value depends on internal JSON type:
     *      - null    - `0`
     *      - boolean - `1`
     *      - string  - `1`
     *      - number  - `1`
     *      - array   - the size() of array
     *      - object  - the size() of object
     */
    size_type size () const noexcept
    {
        size_type result = 1;

        switch (_type) {
            case type_enum::null:
                result = 0;
                break;
            case type_enum::array:
                result = _value.array_value->size();
                break;
            case type_enum::object:
                result = _value.object_value->size();
                break;
            default:
                break;
        }

        return result;
    }

    /**
     * @brief Checks whether the JSON value has no elements.
     * @return The return value depends on internal JSON type:
     *      - null    - @c true
     *      - boolean - @c false
     *      - string  - @c false
     *      - number  - @c false
     *      - array   - @c true if array has no elements, @c false otherwise
     *      - object  - @c true if object has no elements, @c false otherwise
     */
    bool empty () const noexcept
    {
        return size() == 0;
    }

    /**
     * @brief Returns the maximum number of elements the JSON value is able to hold.
     * @return The return value depends on internal JSON type:
     *      - null    - `0`
     *      - boolean - `1`
     *      - string  - `1`
     *      - number  - `1`
     *      - array   - the max_size() of array
     *      - object  - the max_size() of object
     */
    size_type max_size () const noexcept
    {
        size_type result = 1;

        switch (_type) {
            case type_enum::null:
                result = 0;
                break;
            case type_enum::array:
                result = _value.array_value->max_size();
                break;
            case type_enum::object:
                result = _value.object_value->max_size();
                break;
            default:
                break;
        }

        return result;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Collection specific methods
    //
    // Modifiers
    ////////////////////////////////////////////////////////////////////////////
    void clear () noexcept
    {
        switch (_type) {
            case type_enum::boolean:
                _value.boolean_value = boolean_type(false);
                break;

            case type_enum::integer:
                _value.integer_value = integer_type(0);
                break;

            case type_enum::uinteger:
                _value.uinteger_value = uinteger_type(0);
                break;

            case type_enum::real:
                _value.real_value = real_type(0.0);
                break;

            case type_enum::string:
                _value.string_value->clear();
                break;

            case type_enum::object:
                _value.object_value->clear();
                break;

            case type_enum::array:
                _value.array_value->clear();
                break;

            case type_enum::null:
            default:
                break;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Array specific methods
    ////////////////////////////////////////////////////////////////////////////
    void push_back (value && v)
    {
        std::cout << "push_back (value &&): " << to_string(v.type()) << "\n";
    }

    reference operator += (value && v)
    {
        std::cout << "reference operator += (value &&): " << to_string(v.type()) << "\n";
        push_back(std::forward<value>(v));
        return *this;
    }

    void push_back (value const & v)
    {
        std::cout << "push_back (value const &): " << to_string(v.type()) << "\n";
    }

    reference operator += (value const & v)
    {
        std::cout << "reference operator += (value const &): " << to_string(v.type()) << "\n";
        push_back(v);
        return *this;
    }
};

}} // // namespace pfs::json
