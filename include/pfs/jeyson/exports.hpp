////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef JEYSON__STATIC
#   ifndef JEYSON__EXPORT
#       if _MSC_VER
#           if defined(JEYSON__EXPORTS)
#               define JEYSON__EXPORT __declspec(dllexport)
#           else
#               define JEYSON__EXPORT __declspec(dllimport)
#           endif
#       else
#           define JEYSON__EXPORT
#       endif
#   endif
#else
#   define JEYSON__EXPORT
#endif // !JEYSON__STATIC
