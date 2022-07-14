////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "pfs/jeyson/exports.hpp"
#include <memory>
#include <string>
#include <cstdint>
#include <cstring>
#include <utility>

struct json_t;

namespace jeyson {
namespace backend {

struct jansson
{
    using size_type   = std::size_t;
    using string_type = std::string;
    using key_type    = std::string;

#if _MSC_VER
// Eliminate warning:
// C4251: 'jeyson::backend::jansson::index_type::key': class 'std::basic_string<char,std::char_traits<char>,std::allocator<char>>' 
// needs to have dll-interface to be used by clients of union 'jeyson::backend::jansson::index_type'
#   pragma warning(push)
#   pragma warning(disable: 4251)
#endif
    union JEYSON__EXPORT index_type {
        size_type i {0};
        key_type key;

        index_type () {}
        ~index_type () {}
    };
#if _MSC_VER
#   pragma warning(pop)
#endif

    class basic_rep
    {
    public:
        json_t * _ptr {nullptr};
    };

    class JEYSON__EXPORT rep : public basic_rep
    {
    public:
        rep ();
        rep (rep const & other);
        rep (rep && other);
        rep (json_t * p);
        ~rep ();
    };

    class JEYSON__EXPORT ref: public basic_rep
    {
    public:
        json_t * _parent {nullptr};
        index_type _index;

    public:
        ref ();
        ~ref ();

        ref (json_t * ptr, json_t * parent, size_type index);
        ref (json_t * ptr, json_t * parent, std::string const & key);
        ref (ref const &);
        ref (ref &&);
    };
};

}} // namespace jeyson::backend
