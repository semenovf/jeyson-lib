#pragma once
#include "constants.hpp"
#include "error.hpp"
#include "pfs/iterator.hpp"
#include "pfs/variant.hpp"
#include <cassert>

namespace pfs {
namespace json {

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

    using scalar_iterator = pointer_proxy_iterator<JsonValueType>;
    using array_iterator  = typename JsonValueType::array_type::iterator;
    using object_iterator = typename JsonValueType::object_type::iterator;

    enum { scalar_iterator_index, array_iterator_index, object_iterator_index };

    using mixed_iterator = std::variant<
              scalar_iterator
            , array_iterator
            , object_iterator>;

public:
    using pointer = typename base_class::pointer;
    using reference = typename base_class::reference;
    using difference_type = typename base_class::difference_type;

private:
    int _index {-1};
    mixed_iterator _mit;

protected:
    basic_iterator (pointer p) noexcept
        : _index(scalar_iterator_index)
    {
        auto & native_it = std::get<scalar_iterator_index>(_mit);
        native_it = p;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Set begin / end for array value
    ////////////////////////////////////////////////////////////////////////////
    basic_iterator (array_iterator it) noexcept
        : _index(array_iterator_index)
    {
        auto & native_it = std::get<array_iterator_index>(_mit);
        native_it = it;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Set begin / end for object value
    ////////////////////////////////////////////////////////////////////////////
    basic_iterator (object_iterator it) noexcept
        : _index(object_iterator_index)
    {
        auto & native_it = std::get<object_iterator_index>(_mit);
        native_it = it;
    }

    const reference ref () const
    {
        switch (_index) {
            case array_iterator_index:
                return *std::get<array_iterator_index>(_mit);
            case object_iterator_index:
                return std::get<object_iterator_index>(_mit)->second;
            default:
                break;
        }

        return *std::get<scalar_iterator_index>(_mit);
    }

    const pointer ptr () const
    {
        auto & r = ref();
        return & r;
    }

public:
    ////////////////////////////////////////////////////////////////////////////
    // Bidirectional iterator requirements
    ////////////////////////////////////////////////////////////////////////////
    reference ref ()
    {
        switch (_index) {
            case array_iterator_index:
                return *std::get<array_iterator_index>(_mit);
            case object_iterator_index:
                return std::get<object_iterator_index>(_mit)->second;
            default:
                break;
        }

        return *std::get<scalar_iterator_index>(_mit);
    }

    pointer ptr ()
    {
        auto & r = ref();
        return & r;
    }

    void increment (difference_type)
    {
        switch (_index) {
            case scalar_iterator_index:
                ++std::get<scalar_iterator_index>(_mit);
                break;
            case array_iterator_index:
                ++std::get<array_iterator_index>(_mit);
                break;
            case object_iterator_index:
                ++std::get<object_iterator_index>(_mit);
                break;
        }
    }

    void decrement (difference_type)
    {
        switch (_index) {
            case scalar_iterator_index:
                --std::get<scalar_iterator_index>(_mit);
                break;
            case array_iterator_index:
                --std::get<array_iterator_index>(_mit);
                break;
            case object_iterator_index:
                --std::get<object_iterator_index>(_mit);
                break;
        }
    }

    bool equals (basic_iterator const & rhs) const
    {
        if (_index != rhs._index)
            return false;

        return this->ptr() == rhs.ptr();
    }
};

}} // pfs::json
