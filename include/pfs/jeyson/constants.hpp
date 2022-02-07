////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019 Vladislav Trifochkin
//
// This file is part of [pfs-json](https://github.com/semenovf/pfs-json) library.
//
// Changelog:
//      2019.12.17 Initial version
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace pfs {
namespace json {

enum class type_enum : signed char // 8 signed byte
{
      null = 0
    , boolean
    , integer
    , uinteger
    , real
    , string
    , array
    , object
};

template <type_enum I>
inline constexpr int type_index ()
{
    return static_cast<int>(I);
}

}} // namespace pfs::json
