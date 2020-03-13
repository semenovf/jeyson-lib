#pragma once
#include "constants.hpp"
#include "pfs/iterator.hpp"
#include "pfs/variant.hpp"
#include <cassert>

namespace pfs {
namespace json {

// template <typename JsonType>
// class scalar_iterator
//     : public iterator_facade<random_access_iterator_tag
//                 , scalar_iterator<JsonType>
//                 , typename JsonType::value_type
//                 , typename JsonType::pointer
//                 , typename JsonType::reference
//                 , typename JsonType::difference_type> // Distance
// {
//     template <typename BoolT
//         , typename IntT
//         , typename RealT
//         , typename StringT
//         , template <typename> class SequenceContainer
//         , template <typename, typename> class AssociativeContainer>
//     friend class json;
//
// public:
//     typedef random_access_iterator_tag         iterator_category;
//     typedef typename JsonType::value_type      value_type;
//     typedef typename JsonType::difference_type difference_type;
//     typedef typename JsonType::pointer         pointer;
//     typedef typename JsonType::reference       reference;
//
// protected:
//     pointer _pvalue;
//
// protected:
//     scalar_iterator (pointer pvalue)
//         : _pvalue(pvalue)
//     {}
//
// public:
//     static reference ref (scalar_iterator & it)
//     {
//         return *it._pvalue;
//     }
//
//     static pointer ptr (scalar_iterator & it)
//     {
//         return it._pvalue;
//     }
//
//     static void increment (scalar_iterator &, difference_type /*n*/ = 1)
//     {
//         PFS_THROW(runtime_error("json::scalar_iterator::increment()"));
//     }
//
//     static void decrement (scalar_iterator & it, difference_type n = 1)
//     {
//         PFS_THROW(runtime_error("json::scalar_iterator::decrement()"));
//     }
//
//     static bool equals (scalar_iterator const & lhs
//             , scalar_iterator const & rhs)
//     {
//         return lhs._pvalue == rhs._pvalue;
//     }
//
//     static int compare (scalar_iterator const & lhs
//             , scalar_iterator const & rhs)
//     {
//         PFS_THROW(runtime_error("json::scalar_iterator::compare()"));
//     }
//
//     static reference subscript (scalar_iterator & it
//             , difference_type n)
//     {
//         PFS_THROW(runtime_error("json::scalar_iterator::subscript()"));
//     }
//
//     static difference_type diff (scalar_iterator const & lhs
//             , scalar_iterator const & rhs)
//     {
//         PFS_THROW(runtime_error("json::scalar_iterator::diff()"));
//     }
//
// public:
//     scalar_iterator ()
//         : _pvalue(0)
//     {}
//
//     scalar_iterator & operator = (scalar_iterator const & rhs)
//     {
//         if (_pvalue != rhs._pvalue)
//             _pvalue = rhs._pvalue;
//         return *this;
//     }
//
//     template <typename JsonTypeU>
//     friend class scalar_iterator;
//
//     // Allow iterator to const_iterator assignment
// #if __cplusplus >= 201103L
//     template <typename JsonTypeU, typename EnableIf = pfs::enable_if<pfs::is_same<pfs::remove_cv<JsonType>,JsonTypeU>::value> >
//     scalar_iterator & operator = (scalar_iterator<JsonTypeU> const & rhs)
// #else
//     template <typename JsonTypeU>
//     scalar_iterator & operator = (scalar_iterator<JsonTypeU> const & rhs)
// #endif
//     {
//         if (_pvalue != rhs._pvalue)
//             _pvalue = rhs._pvalue;
//         return *this;
//     }
// };
//

using pfs::variant;

template <typename T, type_enum I>
struct iterator_helper
{
    using iterator_impl = pointer_proxy_iterator<T>;
};

template <typename T>
struct iterator_helper<T, type_enum::array>
{
    using iterator_impl = typename T::iterator;
};

template <typename T>
struct iterator_helper<T, type_enum::object>
{
    using iterator_impl = typename T::iterator;
};

template <typename JsonValueType>
class basic_iterator : public iterator_facade<bidirectional_iterator_tag
        , basic_iterator<JsonValueType>
        , JsonValueType
        , JsonValueType *
        , JsonValueType &
        , std::ptrdiff_t>
{
    friend JsonValueType;

    using base_class = iterator_facade<bidirectional_iterator_tag
            , basic_iterator<JsonValueType>
            , JsonValueType
            , JsonValueType *
            , JsonValueType &
            , std::ptrdiff_t>;

    using null_iterator     = pointer_proxy_iterator<std::nullptr_t>;
    using boolean_iterator  = pointer_proxy_iterator<typename JsonValueType::boolean_type>;
    using string_iterator   = pointer_proxy_iterator<typename JsonValueType::string_type>;
    using integer_iterator  = pointer_proxy_iterator<typename JsonValueType::integer_type>;
    using uinteger_iterator = pointer_proxy_iterator<typename JsonValueType::uinteger_type>;
    using real_iterator     = pointer_proxy_iterator<typename JsonValueType::real_type>;
    using array_iterator    = typename JsonValueType::array_type::iterator;
    using object_iterator   = typename JsonValueType::object_type::iterator;

    using mixed_iterator = variant<
              std::nullptr_t
            , boolean_iterator
            , integer_iterator
            , uinteger_iterator
            , real_iterator
            , string_iterator
            , array_iterator
            , object_iterator>;

public:
    using pointer = typename base_class::pointer;
    using reference = typename base_class::reference;
    using difference_type = typename base_class::difference_type;

//     typedef JsonType                    value_type;
//     typedef JsonType *                  pointer;
//     typedef JsonType &                  reference;
//     typedef typename JsonType::key_type key_type;
//     typedef ptrdiff_t                   difference_type;
//
// protected:
//     typedef typename JsonType::object_type           object_type;
//     typedef typename JsonType::array_type::iterator  array_iterator_type;
//     typedef typename JsonType::object_type::iterator object_iterator_type;
//     typedef scalar_iterator<JsonType>                scalar_iterator_type;
//
//     pointer              _pvalue;
//     array_iterator_type  _array_it;
//     object_iterator_type _object_it;
//     scalar_iterator_type _scalar_it;
//
// public:
//     static reference ref (basic_iterator & it);
//     static pointer ptr (basic_iterator & it);
//     static void increment (basic_iterator &, difference_type n = 1);
//
//     static void decrement (basic_iterator & it, difference_type n = 1)
//     {
//         increment(it, -n);
//     }
//
//     static bool equals (basic_iterator const & lhs, basic_iterator const & rhs);
//     static int compare (basic_iterator const & lhs, basic_iterator const & rhs);
//     static reference subscript (basic_iterator & it, difference_type n);
//     static difference_type diff (basic_iterator const & lhs, basic_iterator const & rhs);

private:
    int _index {0};
    mixed_iterator _mit;

protected:
    basic_iterator () noexcept
        : _index(type_index<type_enum::null>())
    {}

    basic_iterator (typename boolean_iterator::pointer p) noexcept
        : _index(type_index<type_enum::boolean>())
    {
        constexpr auto index = type_index<type_enum::boolean>();
        auto & native_it = pfs::get<index>(_mit);
        native_it = boolean_iterator(p);
    }

    basic_iterator (typename integer_iterator::pointer p) noexcept
        : _index(type_index<type_enum::integer>())
    {
        constexpr auto index = type_index<type_enum::integer>();
        auto & native_it = pfs::get<index>(_mit);
        native_it = integer_iterator(p);
    }

    basic_iterator (typename uinteger_iterator::pointer p) noexcept
        : _index(type_index<type_enum::uinteger>())
    {
        constexpr auto index = type_index<type_enum::uinteger>();
        auto & native_it = pfs::get<index>(_mit);
        native_it = uinteger_iterator(p);
    }

    basic_iterator (typename real_iterator::pointer p) noexcept
        : _index(type_index<type_enum::real>())
    {
        constexpr auto index = type_index<type_enum::real>();
        auto & native_it = pfs::get<index>(_mit);
        native_it = real_iterator(p);
    }

    basic_iterator (typename string_iterator::pointer p) noexcept
        : _index(type_index<type_enum::string>())
    {
        constexpr auto index = type_index<type_enum::string>();
        auto & native_it = pfs::get<index>(_mit);
        native_it = string_iterator(p);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Set begin / end for array value
    ////////////////////////////////////////////////////////////////////////////
    basic_iterator (array_iterator it) noexcept
        : _index(type_index<type_enum::array>())
    {
        constexpr auto index = type_index<type_enum::array>();
        auto & native_it = pfs::get<index>(_mit);
        native_it = it;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Set begin / end for object value
    ////////////////////////////////////////////////////////////////////////////
    basic_iterator (object_iterator it) noexcept
        : _index(type_index<type_enum::object>())
    {
        constexpr auto index = type_index<type_enum::object>();
        auto & native_it = pfs::get<index>(_mit);
        native_it = it;
    }

public:

//     basic_iterator ()
//         : _pvalue (0)
//     {}
//
//     basic_iterator (basic_iterator const & rhs);
//     basic_iterator & operator = (basic_iterator const & rhs);
//
//     template <typename JsonTypeU>
//     friend class basic_iterator;
//
//     // Allow iterator to const_iterator conversion
// #if __cplusplus >= 201103L
//     template <typename JsonTypeU, typename EnableIf = pfs::enable_if<pfs::is_same<pfs::remove_cv<JsonType>,JsonTypeU>::value> >
//     basic_iterator (basic_iterator<JsonTypeU> const & rhs)
// #else
//     template <typename JsonTypeU>
//     basic_iterator (basic_iterator<JsonTypeU> const & rhs)
// #endif
//         : _pvalue (rhs._pvalue)
//     {
//         operator = (rhs);
//     }
//
// #if __cplusplus >= 201103L
//     template <typename JsonTypeU, typename EnableIf = pfs::enable_if<pfs::is_same<pfs::remove_cv<JsonType>,JsonTypeU>::value> >
//     basic_iterator & operator = (basic_iterator<JsonTypeU> const & rhs)
// #else
//     template <typename JsonTypeU>
//     basic_iterator & operator = (basic_iterator<JsonTypeU> const & rhs)
// #endif
//     {
//         _pvalue = rhs._pvalue;
//
//         if (_pvalue) {
//             switch (_pvalue->type()) {
//             case data_type::object:
//                 _object_it = rhs._object_it;
//                 break;
//
//             case data_type::array:
//                 _array_it = rhs._array_it;
//                 break;
//
//             default:
//                 _scalar_it = rhs._scalar_it;
//                 break;
//             }
//         }
//
//         return *this;
//     }
//
//     key_type key () const
//     {
//         return _pvalue->type() == data_type::object
//                 ? object_type::key_reference(_object_it)
//                 : key_type();
//     }

// Bidirectional iterator requirements
//     reference ref ()
//     {
//         return *_p;
//     }
//
//     pointer ptr ()
//     {
//         return _p;
//     }
//
//     void increment (difference_type)
//     {
//         ++_p;
//     }
//
//     bool equals (basic_iterator const & rhs) const
//     {
//         return _p == rhs._p;
//     }
//
//     void decrement (difference_type)
//     {
//         --_p;
//     }
};

// template <typename JsonValueType>
// template <>
// inline constexpr void basic_iterator<JsonValueType>::set_begin<type_enum::null> ()
// {}


// template <typename JsonType>
// basic_iterator<JsonType>::basic_iterator (basic_iterator const & rhs)
//     : _pvalue (rhs._pvalue)
// {
//     operator = (rhs);
// }
//
// template <typename JsonType>
// basic_iterator<JsonType> &
// basic_iterator<JsonType>::operator = (basic_iterator const & rhs)
// {
//     if (this != & rhs) {
//
//         _pvalue = rhs._pvalue;
//
//         if (_pvalue) {
//             switch (_pvalue->type()) {
//             case data_type::object:
//                 _object_it = rhs._object_it;
//                 break;
//
//             case data_type::array:
//                 _array_it = rhs._array_it;
//                 break;
//
//             default:
//                 _scalar_it = rhs._scalar_it;
//                 break;
//             }
//         }
//     }
//     return *this;
// }
//
// template <typename JsonType>
// void basic_iterator<JsonType>::__set_begin ()
// {
//     switch (_pvalue->type()) {
// 	case data_type::object:
// 		_object_it = _pvalue->_d.object->begin();
//         break;
//
// 	case data_type::array:
// 		_array_it = _pvalue->_d.array->begin();
//         break;
//
//     default:
//         break;
//     }
// }
//
// template <typename JsonType>
// void basic_iterator<JsonType>::__set_end ()
// {
//     switch (_pvalue->type()) {
// 	case data_type::object:
// 		_object_it = _pvalue->_d.object->end();
//         break;
//
// 	case data_type::array:
// 		_array_it = _pvalue->_d.array->end();
//         break;
//
//     default:
//         break;
//     }
// }
//
// template <typename JsonType>
// typename basic_iterator<JsonType>::reference
// basic_iterator<JsonType>::ref (basic_iterator & it)
// {
//     switch (it._pvalue->type()) {
// 	case data_type::object:
// 		return object_type::mapped_reference(it._object_it); //it._object_it->second;
//
// 	case data_type::array:
// 		return *it._array_it;
//
//     default:
//         break;
//     }
//
//     return *it._scalar_it;
// }
//
// template <typename JsonType>
// typename basic_iterator<JsonType>::pointer
// basic_iterator<JsonType>::ptr (basic_iterator & it)
// {
//     switch (it._pvalue->type()) {
//     case data_type::object:
//         return & object_type::mapped_reference(it._object_it);//->second;
//
//     case data_type::array:
//         return & *it._array_it;
//
//     default:
//         break;
//     }
//
//     return & *it._scalar_it;
// }
//
// template <typename JsonType>
// void basic_iterator<JsonType>::increment (basic_iterator & it, difference_type n)
// {
//     switch (it._pvalue->type()) {
// 	case data_type::object:
//         if (n == 0)
//             ;
//         else if (n == 1)
//             ++it._object_it;
//         else if (n == -1)
//             --it._object_it;
//         else
//             PFS_THROW(out_of_range("json::basic_iterator::increment()"));
//         break;
//
// 	case data_type::array:
// 		it._array_it += n;
//         break;
//
//     default:
//         it._scalar_it += n;
//         break;
//     }
// }
//
// template <typename JsonType>
// bool basic_iterator<JsonType>::equals (basic_iterator const & lhs
//         , basic_iterator const & rhs)
// {
//     switch (lhs._pvalue->type()) {
// 	case data_type::object:
//         return lhs._object_it == rhs._object_it;
//
// 	case data_type::array:
// 		return lhs._array_it == rhs._array_it;
//
//     default:
//         break;
//     }
//
//     return lhs._scalar_it == rhs._scalar_it;
// }
//
// template <typename JsonType>
// int basic_iterator<JsonType>::compare (basic_iterator const & lhs
//         , basic_iterator const & rhs)
// {
//     switch (lhs._pvalue->type()) {
// 	case data_type::object:
//         PFS_THROW(runtime_error("json::basic_iterator::compare(): object"));
//
// 	case data_type::array:
// 		return lhs._array_it - rhs._array_it;
//
//     default:
//         break;
//     }
//
//     return scalar_iterator_type::compare(lhs._scalar_it, rhs._scalar_it);
// }
//
// template <typename JsonType>
// typename basic_iterator<JsonType>::reference
// basic_iterator<JsonType>::subscript (basic_iterator & it, difference_type n)
// {
//     switch (it._pvalue->type()) {
// 	case data_type::object:
//         PFS_THROW(runtime_error("json::basic_iterator::subscript(): object"));
//
// 	case data_type::array:
// 		return it._array_it[n];
//
//     default:
//         break;
//     }
//
//     return scalar_iterator_type::subscript(it._scalar_it, n);
// }
//
// template <typename JsonType>
// typename basic_iterator<JsonType>::difference_type
// basic_iterator<JsonType>::diff (basic_iterator const & lhs
//         , basic_iterator const & rhs)
// {
//     switch (lhs._pvalue->type()) {
// 	case data_type::object:
//         PFS_THROW(runtime_error("json::basic_iterator::diff(): object"));
//
// 	case data_type::array:
// 		return lhs._array_it - rhs._array_it;
//
//     default:
//         break;
//     }
//
//     return scalar_iterator_type::diff(lhs._scalar_it, rhs._scalar_it);
// }

}} // pfs::json
