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
#include "pfs/fmt.hpp"
#include "pfs/jeyson/json.hpp"
#include "pfs/jeyson/backend/jansson.hpp"
#include "pfs/optional.hpp"
#include <vector>

template <typename Backend>
void run_tests ()
{
    using json = jeyson::json<Backend>;

    json::failure = [] (jeyson::error err) {
        fmt::print(stderr, "ERROR: {}\n", err.what());
    };

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
        REQUIRE(is_null(j));
    }

    {
        json j {true};
        REQUIRE(!!j);
        REQUIRE(is_bool(j));
    }

    {
        json j {false};
        REQUIRE(!!j);
        REQUIRE(is_bool(j));
    }

    {
        bool b1 = true;
        bool volatile b2 = b1;
        bool const b3 = b2;
        bool volatile const b4 = b3;
        bool const & b5 = b3;
        bool volatile const & b6 = b5;

        json j1 {b1};
        json j2 {b2};
        json j3 {b3};
        json j4 {b4};
        json j5 {b5};
        json j6 {b6};
        REQUIRE(is_bool(j1));
        REQUIRE(is_bool(j2));
        REQUIRE(is_bool(j3));
        REQUIRE(is_bool(j4));
        REQUIRE(is_bool(j5));
        REQUIRE(is_bool(j6));
    }

    {
        json j {0};
        REQUIRE(!!j);
        REQUIRE(is_integer(j));
    }

    {
        json j {42};
        REQUIRE(!!j);
        REQUIRE(is_integer(j));
    }

    {
        json j {-42};
        REQUIRE(!!j);
        REQUIRE(is_integer(j));
    }

    // Constructors from integral values
    {
        for (auto const & j: std::array<json, 11>{
              json{char{42}}
            , json{std::int8_t{42}}
            , json{std::uint8_t{42}}
            , json{std::int16_t{42}}
            , json{std::uint16_t{42}}
            , json{std::int32_t{42}}
            , json{std::uint32_t{42}}
            , json{std::int64_t{42}}
            , json{std::uint64_t{42}}
            , json{std::intmax_t{42}}
            , json{std::uintmax_t{42}}
        }) {
            REQUIRE(is_integer(j));
        }

        uint16_t i1 = 42;
        uint16_t volatile i2 = i1;
        uint16_t const i3 = i2;
        uint16_t volatile const i4 = i3;
        uint16_t const & i5 = i3;
        uint16_t volatile const & i6 = i5;

        json j1 {i1};
        json j2 {i2};
        json j3 {i3};
        json j4 {i4};
        json j5 {i5};
        json j6 {i6};
        REQUIRE(is_integer(j1));
        REQUIRE(is_integer(j2));
        REQUIRE(is_integer(j3));
        REQUIRE(is_integer(j4));
        REQUIRE(is_integer(j5));
        REQUIRE(is_integer(j6));
    }

    {
        double f = 3.14;
        json j {f};
        REQUIRE(!!j);
        REQUIRE(is_real(j));

        float f1 = 3.14;
        float volatile f2 = f1;
        float const f3 = f2;
        float volatile const f4 = f3;
        float const & f5 = f3;
        float volatile const & f6 = f5;

        json j1 {f1};
        json j2 {f2};
        json j3 {f3};
        json j4 {f4};
        json j5 {f5};
        json j6 {f6};
        REQUIRE(is_real(j1));
        REQUIRE(is_real(j2));
        REQUIRE(is_real(j3));
        REQUIRE(is_real(j4));
        REQUIRE(is_real(j5));
        REQUIRE(is_real(j6));
    }

    {
        json j {std::string{"Hello"}};
        REQUIRE(!!j);
        REQUIRE(is_string(j));
    }

    {
        json j {", World"};
        REQUIRE(!!j);
        REQUIRE(is_string(j));
    }

    {
        json j {"!", 1};
        REQUIRE(!!j);
        REQUIRE(is_string(j));
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
        REQUIRE(is_integer(j1));
        REQUIRE(is_integer(j2));
    }

    {
        json j1 {42};
        json j2 {std::move(j1)};

        REQUIRE_FALSE(!!j1);
        REQUIRE(!!j2);
        REQUIRE(is_integer(j2));
    }

    {
        json j1 {42};
        json j2;

        REQUIRE(!!j1);
        REQUIRE_FALSE(!!j2);

        j2 = j1;
        REQUIRE(!!j1);
        REQUIRE(!!j2);
        REQUIRE(is_integer(j1));
        REQUIRE(is_integer(j2));
    }

    {
        json j1 {42};
        json j2;

        REQUIRE(!!j1);
        REQUIRE_FALSE(!!j2);

        j2 = std::move(j1);
        REQUIRE_FALSE(!!j1);
        REQUIRE(!!j2);
        REQUIRE(is_integer(j2));
    }

    {
        json j;
        j.push_back(json{"Hello"});
        auto ref = j[0];

        json j1{ref};

        REQUIRE(is_array(j));
        REQUIRE(is_string(ref));
        REQUIRE(is_string(j1));
        REQUIRE_EQ(jeyson::get<std::string>(ref), std::string{"Hello"});
        REQUIRE_EQ(jeyson::get<std::string>(j1), std::string{"Hello"});
    }

////////////////////////////////////////////////////////////////////////////////
// Comparison operators
////////////////////////////////////////////////////////////////////////////////
    {
        json j01;
        json j02;
        json j1 {42};
        json j2 {42};
        json j3 {43};

        REQUIRE_EQ(j01, j02);
        REQUIRE_EQ(j1 , j2);
        REQUIRE_NE(j01, j1);
        REQUIRE_NE(j1 , j3);
    }

////////////////////////////////////////////////////////////////////////////////
// Modifiers and Element access
////////////////////////////////////////////////////////////////////////////////
    {
        json j;

        j.push_back(json{nullptr});
        j.push_back(json{true});

        {
            json n {42};
            j.push_back(n);
        }

        {
            json f {3.14};
            j.push_back(std::move(f));
        }

        j.push_back(json{"Hello"});

        REQUIRE_EQ(j.size(), 5);

        REQUIRE(is_null(j[0]));
        REQUIRE(is_bool(j[1]));
        REQUIRE(is_integer(j[2]));
        REQUIRE(is_real(j[3]));
        REQUIRE(is_string(j[4]));

        REQUIRE_EQ(jeyson::get<bool>(j[1]), true);
        REQUIRE_EQ(jeyson::get<int>(j[2]), 42);
        REQUIRE_EQ(jeyson::get<float>(j[3]), float{3.14});
    }

    {
        json j;
        j["KEY1"] = json{42};
        j["KEY2"] = json{"Hello"};

        REQUIRE_EQ(jeyson::get<int>(j["KEY1"]), 42);
        REQUIRE_EQ(jeyson::get<std::string>(j["KEY2"]), std::string{"Hello"});
    }

    {
        json j;
        j.push_back(json{1});
        j.push_back(json{"?"});

        REQUIRE_EQ(jeyson::get<int>(j[0]), 1);
        REQUIRE_EQ(jeyson::get<std::string>(j[1]), std::string{"?"});

        j[0] = json{42};
        j[1] = json{"Hello"};

        REQUIRE_EQ(jeyson::get<int>(j[0]), 42);
        REQUIRE_EQ(jeyson::get<std::string>(j[1]), std::string{"Hello"});
    }

////////////////////////////////////////////////////////////////////////////////
// Stringification
////////////////////////////////////////////////////////////////////////////////
    {
        json j;
        j.push_back(json{nullptr});
        j.push_back(json{true});
        j.push_back(json{42});

        REQUIRE_EQ(to_string(j), std::string{"[null,true,42]"});
    }

////////////////////////////////////////////////////////////////////////////////
// Parsing
////////////////////////////////////////////////////////////////////////////////
    {
        // Good
        {
            std::string s {"[null,true,42]"};
            auto j = json::parse(s);

            REQUIRE(j);

            REQUIRE(is_null(j[0]));
            REQUIRE(is_bool(j[1]));
            REQUIRE(is_integer(j[2]));
            REQUIRE_EQ(jeyson::get<bool>(j[1]), true);
            REQUIRE_EQ(jeyson::get<int>(j[2]), 42);
        }

        // Bad
        {
            std::string s {"[null"};
            auto j = json::parse(s);

            REQUIRE_FALSE(j);
        }

        {
            auto j1 = json::parse(pfs::filesystem::utf8_decode("data/twitter.json"));
            auto j2 = json::parse(pfs::filesystem::utf8_decode("data/canada.json"));
            auto j3 = json::parse(pfs::filesystem::utf8_decode("data/citm_catalog.json"));

            REQUIRE(j1);
            REQUIRE(j2);
            REQUIRE(j3);

            auto code = j1["statuses"][0]["metadata"]["iso_language_code"];
            REQUIRE_EQ(jeyson::get<std::string>(code), std::string{"ja"});
        }
    }
}

template <typename Backend>
void run_custom_test ()
{
    using json = jeyson::json<Backend>;

    json::failure = [] (jeyson::error err) {
        fmt::print(stderr, "ERROR: {}\n", err.what());
    };

    struct A {
        std::vector<pfs::optional<json>> v;
    };

    struct B {
        A d;
    };

    B b1;
    b1.d.v.push_back(json{42});
    b1.d.v.push_back(json{"abc"});

    REQUIRE_FALSE(b1.d.v.empty());
    REQUIRE_EQ(b1.d.v.size(), 2);
    REQUIRE(is_integer(*b1.d.v[0]));
    REQUIRE_EQ(jeyson::get<int>(*b1.d.v[0]), 42);

    B b2;
    b2.d.v.push_back(json{43});
    REQUIRE_FALSE(b2.d.v.empty());
    REQUIRE_EQ(b2.d.v.size(), 1);
    REQUIRE_EQ(jeyson::get<int>(*b2.d.v[0]), 43);

    b1 = std::move(b2);

    REQUIRE(b2.d.v.empty());
    REQUIRE_FALSE(b1.d.v.empty());
    REQUIRE_EQ(b1.d.v.size(), 1);
    REQUIRE_EQ(jeyson::get<int>(*b1.d.v[0]), 43);
}

TEST_CASE("JSON Jansson backend") {
    run_tests<jeyson::jansson_backend>();
    run_custom_test<jeyson::jansson_backend>();
}
