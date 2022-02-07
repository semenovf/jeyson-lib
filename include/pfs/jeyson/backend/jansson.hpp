////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <cstddef>
#include <cstdint>
#include <map>
#include <vector>

struct json_t;

namespace jeyson {

struct jansson_backend
{
    using native_type = json_t *;
};

} // namespace jeyson
