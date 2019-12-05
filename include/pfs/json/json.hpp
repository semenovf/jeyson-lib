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
#include "error.hpp"
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
    using boolean_type  = BoolType;
    using integer_type  = IntegerType;
    using uinteger_type = UIntegerType;
    using real_type     = RealType;
    using string_type   = StringType;
    using array_type    = ArrayType<value>;
    using object_type   = ObjectType<StringType, value>;

    enum class type : signed char // 8 signed byte
    {
          null = 0
        , boolean
        , integer
        , uinteger
        , real
        , string
        , object
        , array
    };

protected:
    type _type;

    union value_rep {
        boolean_type  boolean_value;
        integer_type  integer_value;
        uinteger_type uinteger_value;
        real_type     real_value;
        string_type * string_value;
        array_type *  array_value;
        object_type * object_value;

        value_rep () = default;
        value_rep (boolean_type v) noexcept : boolean_value(v) {}
        value_rep (integer_type v) noexcept : integer_value(v) {}
        value_rep (uinteger_type v) noexcept : uinteger_value(v) {}
        value_rep (real_type v) noexcept : real_value(v) {}
        value_rep (string_type * v) noexcept : string_value(v) {}
        value_rep (array_type * v) noexcept : array_value(v) {}
        value_rep (object_type * v) noexcept : object_value(v) {}

        void destroy (value::type t)
        {
            switch (t) {
                case value::type::string:
                    if (string_value) {
                        delete string_value;
                        string_value = nullptr;
                    }
                    break;

                case value::type::array:
                    if (array_value) {
                        delete array_value;
                        array_value = nullptr;
                    }
                    break;

                case value::type::object:
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
    value (std::nullptr_t = nullptr) : _type(type::null)
    {}

    value (bool v) : _type(type::boolean), _value(boolean_type{v})
    {}

    template <typename T
            , typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type = 0>
    value (T v) : _type(type::integer), _value(integer_type{v})
    {}

    template <typename T
            , typename std::enable_if<std::is_integral<T>::value && !std::is_signed<T>::value, int>::type = 0>
    value (T v) : _type(type::uinteger), _value(uinteger_type{v})
    {}

    template <typename T
            , typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
    value (T v) : _type(type::real), _value(real_type{v})
    {}

    value (StringType const & v)
        : _type(type::string)
        , _value(new StringType{v})
    {}

    /**
     */
    value (StringType && v)
        : _type(type::string)
        , _value(new StringType{std::forward<StringType>(v)})
    {}

    /**
     * C-string must be converted to @c string_type
     */
    value (char const * v)
        : _type(type::string)
        , _value(new StringType{v})
    {}

    /**
     */
    ~value ()
    {
        _value.destroy(_type);
    }

    /**
     */
    bool is_null () const noexcept
    {
        return _type == type::null;
    }

    /**
     */
    bool is_boolean () const noexcept
    {
        return _type == type::boolean;
    }

    /**
     */
    bool is_integer () const noexcept
    {
        return _type == type::integer;
    }

    /**
     */
    bool is_uinteger () const noexcept
    {
        return _type == type::uinteger;
    }

    /**
     */
    bool is_real () const noexcept
    {
        return _type == type::real;
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
        return _type == type::string;
    }

    /**
     */
    bool is_array () const noexcept
    {
        return _type == type::array;
    }

    /**
     */
    bool is_object () const noexcept
    {
        return _type == type::object;
    }
};

}} // // namespace pfs::json
