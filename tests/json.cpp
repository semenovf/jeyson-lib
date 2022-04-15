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
void run_basic_tests ()
{
    using json = jeyson::json<Backend>;

////////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////////
    {
        json j;
        CHECK_FALSE(j);
    }

    {
        json j {nullptr};
        CHECK(j);
        CHECK(is_null(j));
    }

    {
        json j {true};
        CHECK(j);
        CHECK(is_bool(j));
    }

    {
        json j {false};
        CHECK(j);
        CHECK(is_bool(j));
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
        CHECK(is_bool(j1));
        CHECK(is_bool(j2));
        CHECK(is_bool(j3));
        CHECK(is_bool(j4));
        CHECK(is_bool(j5));
        CHECK(is_bool(j6));
        CHECK(is_scalar(j6));
    }

    {
        json j {0};
        CHECK(j);
        CHECK(is_integer(j));
    }

    {
        json j {42};
        CHECK(j);
        CHECK(is_integer(j));
    }

    {
        json j {-42};
        CHECK(j);
        CHECK(is_integer(j));
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
            CHECK(is_integer(j));
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
        CHECK(is_integer(j1));
        CHECK(is_integer(j2));
        CHECK(is_integer(j3));
        CHECK(is_integer(j4));
        CHECK(is_integer(j5));
        CHECK(is_integer(j6));
        CHECK(is_scalar(j6));
    }

    {
        double f = 3.14;
        json j {f};
        CHECK(!!j);
        CHECK(is_real(j));

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
        CHECK(is_real(j1));
        CHECK(is_real(j2));
        CHECK(is_real(j3));
        CHECK(is_real(j4));
        CHECK(is_real(j5));
        CHECK(is_real(j6));
        CHECK(is_scalar(j6));
    }

    {
        json j {std::string{"Hello"}};
        CHECK(!!j);
        CHECK(is_string(j));
        CHECK(is_scalar(j));
    }

    {
        json j {", World"};
        CHECK(j);
        CHECK(is_string(j));
        CHECK(is_scalar(j));
    }

    {
        json j {"!", 1};
        CHECK(j);
        CHECK(is_string(j));
        CHECK(is_scalar(j));
    }

    // Construct from unitialized value
    {
        json j1;
        json j2 {j1};

        CHECK_FALSE(j1);
        CHECK_FALSE(j2);
    }

    // Copy constructor
    {
        json j1 {42};
        json j2 {j1};

        CHECK(j1);
        CHECK(j2);
        CHECK(is_integer(j1));
        CHECK(is_integer(j2));
    }

    // Move constructor
    {
        json j1 {42};
        json j2 {std::move(j1)};

        CHECK_FALSE(!!j1);
        CHECK(!!j2);
        CHECK(is_integer(j2));
    }

    // Copy assignment
    {
        json j1 {42};
        json j2;

        CHECK(!!j1);
        CHECK_FALSE(!!j2);

        j2 = j1;
        CHECK(!!j1);
        CHECK(!!j2);
        CHECK(is_integer(j1));
        CHECK(is_integer(j2));
    }

    // Move assignment
    {
        json j1 {42};
        json j2;

        CHECK(!!j1);
        CHECK_FALSE(!!j2);

        j2 = std::move(j1);
        CHECK_FALSE(!!j1);
        CHECK(!!j2);
        CHECK(is_integer(j2));
    }

    // Custom assignment
    {
       json j;

       CHECK_FALSE(j);

       j = nullptr;

       CHECK(is_null(j));
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

        CHECK(j01 == j02);
        CHECK(j1 == j2);
        CHECK(j01 != j1);
        CHECK(j1 != j3);
    }

////////////////////////////////////////////////////////////////////////////////
// Modifiers and Capacity
////////////////////////////////////////////////////////////////////////////////
    {
        json j;
        j.insert("key1", 42);
        j.insert("key2", 43);

        CHECK_EQ(j.size(), 2);

        j.insert("key3", 44);

        CHECK_EQ(j.size(), 3);

        // Replaced
        j.insert("key3", 45);
        CHECK_EQ(j.size(), 3);
        CHECK(is_structured(j));
    }

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
            CHECK_FALSE(f);
        }

        j.push_back(json{"Hello"});

        REQUIRE_FALSE(j.empty());
        REQUIRE_EQ(j.size(), 5);
        CHECK(is_structured(j));
    }

    // Swap
    {
        json j1;
        json j2{42};

        CHECK(!j1);
        CHECK(j2);

        swap(j1, j2);

        CHECK(j1);
        CHECK(!j2);
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
}

template <typename Backend>
void run_access_tests ()
{
    using json = jeyson::json<Backend>;

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

        j.push_back("Hello");

        REQUIRE_EQ(j.size(), 5);

        auto r1 = j[0];

        CHECK(r1);

        CHECK(j[0].is_null());
        CHECK(j[1].is_bool());
        CHECK(j[2].is_integer());
        CHECK(j[3].is_real());
        CHECK(j[4].is_string());

        auto r_new = j[j.size()];
        CHECK(r_new);
        CHECK(j[5].is_null());

        REQUIRE_THROWS(j.at(j.size()));

//         REQUIRE_EQ(jeyson::get<bool>(j[1]), true);
//         REQUIRE_EQ(jeyson::get<int>(j[2]), 42);
//         REQUIRE_EQ(jeyson::get<float>(j[3]), float{3.14});
    }

    {
        json j;
        j.insert("null", nullptr);
        j.insert("bool", true);
        j.insert("int", 43);
        j.insert("real", 3.14159);
        j.insert("string", "hello");

        CHECK(is_object(j));
        CHECK_EQ(j.size(), 5);

        CHECK(is_null(j["null"]));
        CHECK(is_bool(j["bool"]));
        CHECK(is_integer(j["int"]));
        CHECK(is_real(j["real"]));
        CHECK(is_string(j["string"]));
    }

//     {
//         json j;
//         j["KEY1"] = json{42};
//         j["KEY2"] = json{"Hello"};
//
//         REQUIRE_EQ(jeyson::get<int>(j["KEY1"]), 42);
//         REQUIRE_EQ(jeyson::get<std::string>(j["KEY2"]), std::string{"Hello"});
//     }

//     {
//         json j;
//         j.push_back(json{1});
//         j.push_back(json{"?"});
//
//         REQUIRE_EQ(jeyson::get<int>(j[0]), 1);
//         REQUIRE_EQ(jeyson::get<std::string>(j[1]), std::string{"?"});
//
//         j[0] = json{42};
//         j[1] = json{"Hello"};
//
//         REQUIRE_EQ(jeyson::get<int>(j[0]), 42);
//         REQUIRE_EQ(jeyson::get<std::string>(j[1]), std::string{"Hello"});
//     }
}

template <typename Backend>
void run_assignment_tests ()
{
    using json = jeyson::json<Backend>;

    // Assing to invalid JSON (non-initialized)
    {
        json j;
        j = nullptr;

        CHECK(is_null(j));

        j = true;

        CHECK(is_bool(j));
//         CHECK_EQ(jeyson::get<bool>(j), true);
    }
}


template <typename Backend>
void run_parsing_tests ()
{
    using json = jeyson::json<Backend>;

    // Good
    {
        std::string s {"[null,true,42]"};
        auto j = json::parse(s);

        REQUIRE(j);
        REQUIRE(is_array(j));
        REQUIRE_EQ(j.size(), 3);

        REQUIRE(j[0].is_null());
        REQUIRE(j[1].is_bool());
        REQUIRE(j[2].is_integer());
//         REQUIRE_EQ(jeyson::get<bool>(j[1]), true);
//         REQUIRE_EQ(jeyson::get<int>(j[2]), 42);
    }

    // Bad
    {
        std::string s {"[null"};

        REQUIRE_THROWS(json::parse(s));
    }

    {
        auto j1 = json::parse(pfs::filesystem::utf8_decode("data/twitter.json"));
        auto j2 = json::parse(pfs::filesystem::utf8_decode("data/canada.json"));
        auto j3 = json::parse(pfs::filesystem::utf8_decode("data/citm_catalog.json"));

        REQUIRE(j1);
        REQUIRE(j2);
        REQUIRE(j3);

        auto code = j1["statuses"][0]["metadata"]["iso_language_code"];
        REQUIRE(code.is_string());
        //REQUIRE_EQ(jeyson::get<std::string>(code), std::string{"ja"});
    }
}

TEST_CASE("JSON Jansson backend") {
    run_basic_tests<jeyson::backend::jansson>();
    run_assignment_tests<jeyson::backend::jansson>();
    run_access_tests<jeyson::backend::jansson>();
    run_parsing_tests<jeyson::backend::jansson>();
}
