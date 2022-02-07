////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019-2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2019.12.17 Initial version (pfs-json).
//      2022.02.07 Initial version (jeyson-lib).
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace jeyson {
namespace v1 {

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

}} // namespace jeyson::v1
