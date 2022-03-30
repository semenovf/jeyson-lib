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

struct json_t;

namespace jeyson {

struct jansson_backend
{
    using size_type   = std::size_t;
    using string_type = std::string;
    using key_type    = std::string;

    struct refdata_type {
        union index_type {
            size_type i;
            std::string key;

            index_type () {};
            ~index_type () {};
        } index;

        json_t * parent {nullptr};

        refdata_type () = default;
        ~refdata_type () = default;
    };

    struct rep_type {
        json_t * ptr {nullptr};
        std::shared_ptr<refdata_type> refdata;

        rep_type ();
        rep_type (rep_type const & other);
        rep_type (rep_type && other);
        rep_type (json_t * p);
        rep_type (json_t * p, json_t * parent, size_type index);
        rep_type (json_t * p, json_t * parent, std::string const & key);
        ~rep_type ();
    };
};

} // namespace jeyson
