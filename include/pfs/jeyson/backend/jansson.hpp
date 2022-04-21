////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////
#pragma once
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

    class rep
    {
    public:
        json_t * _ptr {nullptr};

    public:
        rep ();
        rep (rep const & other);
        rep (rep && other);
        rep (json_t * p);
        ~rep ();
    };

    class ref: public rep
    {
    public:
        json_t * _parent {nullptr};

        union index_type {
            size_type i;
            key_type key;

            index_type () {}
            ~index_type () {}
        } _index;

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
