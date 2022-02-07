////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/jeyson/json.hpp"
#include "pfs/jeyson/backend/jansson.hpp"

template <typename Backend>
void run_tests ()
{
    using json = jeyson::json<Backend>;

////////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////////
    {
        json j;
        REQUIRE_FALSE(!!j);
    }

    {
        json j {nullptr};
        REQUIRE(!!j);
        REQUIRE(j.is_null());
    }

    {
        json j {true};
        REQUIRE(!!j);
        REQUIRE(j.is_bool());
    }

    {
        json j {false};
        REQUIRE(!!j);
        REQUIRE(j.is_bool());
    }

    {
        json j {0};
        REQUIRE(!!j);
        REQUIRE(j.is_integer());
    }

    {
        json j {42};
        REQUIRE(!!j);
        REQUIRE(j.is_integer());
    }

    {
        json j {-42};
        REQUIRE(!!j);
        REQUIRE(j.is_integer());
    }

    {
        json j {3.14};
        REQUIRE(!!j);
        REQUIRE(j.is_real());
    }

    {
        json j {std::string{"Hello"}};
        REQUIRE(!!j);
        REQUIRE(j.is_string());
    }

    {
        json j {", World"};
        REQUIRE(!!j);
        REQUIRE(j.is_string());
    }

    {
        json j {"!", 1};
        REQUIRE(!!j);
        REQUIRE(j.is_string());
    }

    {
        json j1;
        json j2 {j1};

        REQUIRE_FALSE(!!j1);
        REQUIRE_FALSE(!!j2);
    }

    {
        json j1 {42};
        json j2 {j1};

        REQUIRE(!!j1);
        REQUIRE(!!j2);
        REQUIRE(j1.is_integer());
        REQUIRE(j2.is_integer());
    }

    {
        json j1 {42};
        json j2 {std::move(j1)};

        REQUIRE_FALSE(!!j1);
        REQUIRE(!!j2);
        REQUIRE(j2.is_integer());
    }

    {
        json j1 {42};
        json j2;

        REQUIRE(!!j1);
        REQUIRE_FALSE(!!j2);

        j2 = j1;
        REQUIRE(!!j1);
        REQUIRE(!!j2);
        REQUIRE(j1.is_integer());
        REQUIRE(j2.is_integer());
    }

    {
        json j1 {42};
        json j2;

        REQUIRE(!!j1);
        REQUIRE_FALSE(!!j2);

        j2 = std::move(j1);
        REQUIRE_FALSE(!!j1);
        REQUIRE(!!j2);
        REQUIRE(j2.is_integer());
    }
}

TEST_CASE("JSON Jansson backend") {
    run_tests<jeyson::jansson_backend>();
}
