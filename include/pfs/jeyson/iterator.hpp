////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020-2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2020.03.13 Initial version (pfs-json).
//      2022.02.07 Initial version (jeyson-lib).
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "backend/jansson.hpp"
#include "pfs/jeyson/error.hpp"
#include "pfs/iterator.hpp"
#include "pfs/variant.hpp"

namespace jeyson {

template <typename Backend>
class json;

// TODO Implement
// template <typename IteratorBackend>
// class basic_iterator : public pfs::iterator_facade<pfs::bidirectional_iterator_tag
//     , basic_iterator<json<>>
//     , JsonType
//     , JsonType *
//     , JsonType &
//     , std::ptrdiff_t>
// {
// //     friend JsonType;
//
//     using base_class = pfs::iterator_facade<pfs::bidirectional_iterator_tag
//         , basic_iterator<JsonType>
//         , JsonType
//         , JsonType *
//         , JsonType &
//         , std::ptrdiff_t>;
//
//     using scalar_iterator = pfs::pointer_proxy_iterator<JsonType>;
//     using array_iterator  = typename JsonType::array_type::iterator;
//     using object_iterator = typename JsonValueType::object_type::iterator;
//
//     enum { scalar_iterator_index, array_iterator_index, object_iterator_index };
//
//     using mixed_iterator = pfs::variant<
//               scalar_iterator
//             , array_iterator
//             , object_iterator>;
//
// public:
//     using pointer = typename base_class::pointer;
//     using reference = typename base_class::reference;
//     using difference_type = typename base_class::difference_type;
//
// private:
//     int _index {-1};
//     mixed_iterator _mit;
//
// protected:
//     basic_iterator (pointer p) noexcept
//         : _index(scalar_iterator_index)
//     {
//         auto & native_it = pfs::get<scalar_iterator_index>(_mit);
//         native_it = p;
//     }
//
//     ////////////////////////////////////////////////////////////////////////////
//     // Set begin / end for array value
//     ////////////////////////////////////////////////////////////////////////////
//     basic_iterator (array_iterator it) noexcept
//         : _index(array_iterator_index)
//     {
//         auto & native_it = pfs::get<array_iterator_index>(_mit);
//         native_it = it;
//     }
//
//     ////////////////////////////////////////////////////////////////////////////
//     // Set begin / end for object value
//     ////////////////////////////////////////////////////////////////////////////
//     basic_iterator (object_iterator it) noexcept
//         : _index(object_iterator_index)
//     {
//         auto & native_it = pfs::get<object_iterator_index>(_mit);
//         native_it = it;
//     }
//
//     const reference ref () const
//     {
//         switch (_index) {
//             case array_iterator_index:
//                 return *pfs::get<array_iterator_index>(_mit);
//             case object_iterator_index:
//                 return pfs::get<object_iterator_index>(_mit)->second;
//             default:
//                 break;
//         }
//
//         return *pfs::get<scalar_iterator_index>(_mit);
//     }
//
//     const pointer ptr () const
//     {
//         auto & r = ref();
//         return & r;
//     }
//
// public:
//     ////////////////////////////////////////////////////////////////////////////
//     // Bidirectional iterator requirements
//     ////////////////////////////////////////////////////////////////////////////
//     reference ref ()
//     {
//         switch (_index) {
//             case array_iterator_index:
//                 return *pfs::get<array_iterator_index>(_mit);
//             case object_iterator_index:
//                 return pfs::get<object_iterator_index>(_mit)->second;
//             default:
//                 break;
//         }
//
//         return *pfs::get<scalar_iterator_index>(_mit);
//     }
//
//     pointer ptr ()
//     {
//         auto & r = ref();
//         return & r;
//     }
//
//     void increment (difference_type)
//     {
//         switch (_index) {
//             case scalar_iterator_index:
//                 ++pfs::get<scalar_iterator_index>(_mit);
//                 break;
//             case array_iterator_index:
//                 ++pfs::get<array_iterator_index>(_mit);
//                 break;
//             case object_iterator_index:
//                 ++pfs::get<object_iterator_index>(_mit);
//                 break;
//         }
//     }
//
//     void decrement (difference_type)
//     {
//         switch (_index) {
//             case scalar_iterator_index:
//                 --pfs::get<scalar_iterator_index>(_mit);
//                 break;
//             case array_iterator_index:
//                 --pfs::get<array_iterator_index>(_mit);
//                 break;
//             case object_iterator_index:
//                 --pfs::get<object_iterator_index>(_mit);
//                 break;
//         }
//     }
//
//     bool equals (basic_iterator const & rhs) const
//     {
//         if (_index != rhs._index)
//             return false;
//
//         return this->ptr() == rhs.ptr();
//     }
// };

} // namespace jeyson
