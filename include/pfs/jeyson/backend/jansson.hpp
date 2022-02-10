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
#include <cstdint>

struct json_t;

namespace jeyson {

struct jansson_backend
{
    using size_type       = std::size_t;
    using string_type     = std::string;
    using key_type        = std::string;

    struct rep_type {
        json_t * parent {nullptr}; // non-nullptr implies a reference
        json_t * ptr {nullptr};

        rep_type () = default;
        rep_type (rep_type const & other)
            : parent(other.parent)
            , ptr(other.ptr)
        {}

        rep_type (rep_type && other)
            : parent(other.parent)
            , ptr(other.ptr)
        {
            other.parent = nullptr;
            other.ptr = nullptr;
        }

        rep_type (json_t * p, json_t * par)
            : parent(par)
            , ptr(p)
        {}
    };
};

} // namespace jeyson
