////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2022.02.07 Initial version.
////////////////////////////////////////////////////////////////////////////////

// Avoid warning C4996:
// 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead.
#if _MSC_VER
#   define _CRT_SECURE_NO_WARNINGS
#endif

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/filesystem.hpp"
#include "pfs/fmt.hpp"
#include "pfs/jeyson/json.hpp"
#include "pfs/jeyson/backend/jansson.hpp"
#include "pfs/optional.hpp"
#include <array>
#include <vector>

namespace fs = pfs::filesystem;

template <typename Backend>
void run_basic_tests ()
{
    using json = jeyson::json<Backend>;
    using json_ref = jeyson::json_ref<Backend>;

////////////////////////////////////////////////////////////////////////////////
// Constructors
////////////////////////////////////////////////////////////////////////////////
    {
        json j;
        json_ref jr {j};
        CHECK_FALSE(j);
        CHECK_FALSE(jr);
    }

    {
        json j {nullptr};
        json_ref jr {j};
        CHECK(j);
        CHECK(jr);

        CHECK(is_null(j));
        CHECK(is_null(jr));
    }

    {
        json j {true};
        json_ref jr {j};

        CHECK(j);
        CHECK(jr);

        CHECK(is_bool(j));
        CHECK(is_bool(jr));
    }

    {
        json j {false};
        json_ref jr {j};

        CHECK(j);
        CHECK(jr);

        CHECK(is_bool(j));
        CHECK(is_bool(jr));
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
        for (auto const & j: {
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

        float f1 = 3.14f;
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

void run_decoder_tests ()
{
    // nullptr_t
    {
        bool success = true;
        jeyson::decoder<std::nullptr_t> cast;

        CHECK_EQ(cast(nullptr, & success), nullptr);
        CHECK_EQ(cast(true, & success), nullptr);
        CHECK_EQ(cast(false, & success), nullptr);
        CHECK_EQ(cast(std::intmax_t{0}, & success), nullptr);
        CHECK_EQ(cast(std::intmax_t{42}, & success), nullptr);
        CHECK_EQ(cast(std::intmax_t{-42}, & success), nullptr);
        CHECK_EQ(cast(0.0, & success), nullptr);
        CHECK_EQ(cast(3.14159, & success), nullptr);
        CHECK_EQ(cast("", & success), nullptr);
        CHECK_EQ(cast("hello", & success), nullptr);
        CHECK_EQ(cast(0, true, & success), nullptr);
        CHECK_EQ(cast(0, false, & success), nullptr);
        CHECK_EQ(cast(42, true, & success), nullptr);
        CHECK_EQ(cast(42, false, & success), nullptr);
    }

    // bool
    {
        bool success = true;
        jeyson::decoder<bool> cast;

        CHECK_EQ(cast(nullptr, & success), false);
        CHECK_EQ(cast(true, & success), true);
        CHECK_EQ(cast(false, & success), false);
        CHECK_EQ(cast(std::intmax_t{0}, & success), false);
        CHECK_EQ(cast(std::intmax_t{42}, & success), true);
        CHECK_EQ(cast(std::intmax_t{-42}, & success), true);
        CHECK_EQ(cast(0.0, & success), false);
        CHECK_EQ(cast(3.14159, & success), true);
        CHECK_EQ(cast("", & success), false);
        CHECK_EQ(cast("true", & success), true);
        CHECK_EQ(cast("TRUE", & success), true);
        CHECK_EQ(cast("TrUe", & success), true);
        CHECK_EQ(cast("On", & success), true);
        CHECK_EQ(cast("YeS", & success), true);
        CHECK_EQ(cast(0, true, & success), false);
        CHECK_EQ(cast(0, false, & success), false);
        CHECK_EQ(cast(42, true, & success), true);
        CHECK_EQ(cast(42, false, & success), true);

        success = true;

        CHECK_EQ(cast("hello", & success), false);
        CHECK_EQ(success, false);
    }

    // std::intmax_t
    {
        bool success = true;
        jeyson::decoder<std::intmax_t> cast;

        CHECK_EQ(cast(nullptr, & success), 0);
        CHECK_EQ(cast(true, & success), 1);
        CHECK_EQ(cast(false, & success), 0);
        CHECK_EQ(cast(std::intmax_t{0}, & success), 0);
        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
        CHECK_EQ(cast(std::intmax_t{-42}, & success), -42);
        CHECK_EQ(cast(0.0, & success), 0);
        CHECK_EQ(cast(3.14159, & success), 3);
        CHECK_EQ(cast("42", & success), 42);
        CHECK_EQ(cast("-42", & success), -42);
        CHECK_EQ(cast(0, true, & success), 0);
        CHECK_EQ(cast(0, false, & success), 0);
        CHECK_EQ(cast(42, true, & success), 42);
        CHECK_EQ(cast(42, false, & success), 42);

        success = true;
        CHECK_EQ(cast(std::numeric_limits<double>::max(), & success), 0);
        CHECK_EQ(success, false);

        success = true;
        CHECK_EQ(cast(-std::numeric_limits<double>::max(), & success), 0);
        CHECK_EQ(success, false);

        success = true;
        CHECK_EQ(cast("", & success), 0);
        CHECK_EQ(success, false);

        success = true;
        CHECK_EQ(cast("99999999999999999999999999999999999", & success), 0);
        CHECK_EQ(success, false);

        success = true;
        CHECK_EQ(cast("x", & success), 0);
        CHECK_EQ(success, false);
    }

    // char
    {
        bool success = true;
        jeyson::decoder<char> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);

        success = true;
        CHECK_EQ(cast(std::numeric_limits<std::intmax_t>::max(), & success), 0);
        CHECK_FALSE(success); // overflow

        success = true;
        CHECK_EQ(cast(std::numeric_limits<std::intmax_t>::min(), & success), 0);
        CHECK_FALSE(success); // underflow
    }

    // signed char
    {
        bool success = true;
        jeyson::decoder<signed char> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }

    // unsigned char
    {
        bool success = true;
        jeyson::decoder<unsigned char> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }

    // short
    {
        bool success = true;
        jeyson::decoder<short> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }

    // unsigned short
    {
        bool success = true;
        jeyson::decoder<unsigned short> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }

    // int
    {
        bool success = true;
        jeyson::decoder<int> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }

    // unsigned int
    {
        bool success = true;
        jeyson::decoder<unsigned int> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }

    // long
    {
        bool success = true;
        jeyson::decoder<long> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }

    // unsigned long
    {
        bool success = true;
        jeyson::decoder<unsigned long> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }

#if defined(LLONG_MAX)
    // long long
    {
        bool success = true;
        jeyson::decoder<long long> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }

    // unsigned long long
    {
        bool success = true;
        jeyson::decoder<unsigned long long> cast;

        CHECK_EQ(cast(std::intmax_t{42}, & success), 42);
    }
#endif

    // double
    {
        bool success = true;
        jeyson::decoder<double> cast;

        CHECK_EQ(cast(nullptr, & success), 0.0);
        CHECK_EQ(cast(true, & success), 1.0);
        CHECK_EQ(cast(false, & success), 0.0);
        CHECK_EQ(cast(std::intmax_t{0}, & success), 0.0);
        CHECK_EQ(cast(std::intmax_t{42}, & success), 42.0);
        CHECK_EQ(cast(std::intmax_t{-42}, & success), -42.0);
        CHECK_EQ(cast(0.0, & success), 0.0);
        CHECK_EQ(cast(3.14159, & success), 3.14159);
        CHECK_EQ(cast("42", & success), 42.0);
        CHECK_EQ(cast("-42", & success), -42.0);
        CHECK_EQ(cast(0, true, & success), 0.0);
        CHECK_EQ(cast(0, false, & success), 0.0);
        CHECK_EQ(cast(42, true, & success), 42.0);
        CHECK_EQ(cast(42, false, & success), 42.0);
    }

    // std::string
    {
        bool success = true;
        jeyson::decoder<std::string> cast;

        CHECK_EQ(cast(nullptr, & success), "");
        CHECK_EQ(cast(true, & success), "true");
        CHECK_EQ(cast(false, & success), "false");
        CHECK_EQ(cast(std::intmax_t{0}, & success), "0");
        CHECK_EQ(cast(std::intmax_t{42}, & success), "42");
        CHECK_EQ(cast(std::intmax_t{-42}, & success), "-42");

        char buf[128];
        std::sprintf(buf, "%f", 0.0);

        CHECK_EQ(cast(0.0, & success), buf);

        std::sprintf(buf, "%f", 3.14159);

        CHECK_EQ(cast(3.14159, & success), buf);

        CHECK_EQ(cast("42", & success), "42");
        CHECK_EQ(cast("-42", & success), "-42");
        CHECK_EQ(cast(0, true, & success), "0");
        CHECK_EQ(cast(0, false, & success), "0");
        CHECK_EQ(cast(42, true, & success), "42");
        CHECK_EQ(cast(42, false, & success), "42");
    }
}

void run_encoder_tests ()
{
    // TODO
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

        REQUIRE_EQ(jeyson::get<std::nullptr_t>(j[0]), nullptr);
        REQUIRE_EQ(jeyson::get<bool>(j[1]), true);
        REQUIRE_EQ(jeyson::get<int>(j[2]), 42);
        REQUIRE_EQ(jeyson::get<char>(j[2]), 42);
        REQUIRE_EQ(jeyson::get<float>(j[3]), float{3.14});
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
}

template <typename Backend, typename Int>
void check_integral_assigment ()
{
    using json = jeyson::json<Backend>;

    {
        bool success = true;
        json j;
        j = Int{42};

        CHECK(j.is_integer());
        CHECK_EQ(j.template get<Int>(), Int{42});
        CHECK_EQ(j.template get<Int>(success), Int{42});
        CHECK_EQ(j.template get_or<Int>(42), Int{42});

        CHECK_EQ(jeyson::get<Int>(j), Int{42});
    }

    {
        json j;

        j.push_back(nullptr);
        j.push_back(false);
        j.push_back(true);
        j.push_back(42);
        j.push_back(3.14159);
        j.push_back("hello");

        CHECK(j[0].is_null());
        CHECK(j[1].is_bool());
        CHECK(j[2].is_bool());
        CHECK(j[3].is_integer());
        CHECK(j[4].is_real());
        CHECK(j[5].is_string());

        j[0] = 42;
        j[1] = 42;
        j[2] = 42;
        j[3] = 42;
        j[4] = 42;
        j[5] = 42;

        CHECK(j[0].is_integer());
        CHECK(j[1].is_integer());
        CHECK(j[2].is_integer());
        CHECK(j[3].is_integer());
        CHECK(j[4].is_integer());
        CHECK(j[5].is_integer());

        CHECK_EQ(j[0].template get<int>(), 42);
        CHECK_EQ(j[1].template get<int>(), 42);
        CHECK_EQ(j[2].template get<int>(), 42);
        CHECK_EQ(j[3].template get<int>(), 42);
        CHECK_EQ(j[4].template get<int>(), 42);
        CHECK_EQ(j[5].template get<int>(), 42);
    }

    {
        json j;

        j["null"]   = nullptr;
        j["false"]  = false;
        j["true"]   = true;
        j["int"]    = 42;
        j["real"]   = 3.14159;
        j["string"] = "hello";

        CHECK(j["null"].is_null());
        CHECK(j["false"].is_bool());
        CHECK(j["true"].is_bool());
        CHECK(j["int"].is_integer());
        CHECK(j["real"].is_real());
        CHECK(j["string"].is_string());
    }
}

template <typename Backend, typename Float>
void check_floating_point_assigment ()
{
    using json = jeyson::json<Backend>;

    {
        json j;
        j = Float{42};

        CHECK(j.is_real());
        CHECK_EQ(jeyson::get<Float>(j), Float{42});
    }
}

template <typename Backend>
void run_assignment_tests ()
{
    using json = jeyson::json<Backend>;

    {
        json j;
        j = nullptr;

        CHECK(j.is_null());
    }

    {
        json j;
        j = false;

        CHECK(j.is_bool());
        CHECK_EQ(jeyson::get<bool>(j), false);
    }

    {
        json j;
        j = true;

        CHECK(j.is_bool());
        CHECK_EQ(jeyson::get<bool>(j), true);
    }

    check_integral_assigment<Backend, char>();
    check_integral_assigment<Backend, signed char>();
    check_integral_assigment<Backend, unsigned char>();
    check_integral_assigment<Backend, short>();
    check_integral_assigment<Backend, unsigned short>();
    check_integral_assigment<Backend, int>();
    check_integral_assigment<Backend, unsigned int>();
    check_integral_assigment<Backend, long>();
    check_integral_assigment<Backend, unsigned long>();

#if defined(LLONG_MAX)
    check_integral_assigment<Backend, long long>();
    check_integral_assigment<Backend, unsigned long long>();
#endif

    check_floating_point_assigment<Backend, double>();
    check_floating_point_assigment<Backend, float>();


    {
        json j;
        j["KEY1"] = json{42};
        j["KEY2"] = json{"Hello"};

        auto n = jeyson::get<int>(j["KEY1"]);

        CHECK_EQ(jeyson::get<int>(j["KEY1"]), 42);
        CHECK_EQ(jeyson::get<std::string>(j["KEY2"]), std::string{"Hello"});

        json j1 {j["KEY1"]};
        json j2 {j["KEY2"]};
        CHECK_EQ(jeyson::get<int>(j1), 42);
        CHECK_EQ(jeyson::get<std::string>(j2), std::string{"Hello"});
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

    // Copy assignment reference from reference;
    {
        json j;
        j["One"] = 1;
        j["Two"] = 2;

        CHECK_EQ(jeyson::get<int>(j["One"]), 1);
        CHECK_EQ(jeyson::get<int>(j["Two"]), 2);

        j["One"] = j["Two"];

        CHECK_EQ(jeyson::get<int>(j["One"]), 2);
        CHECK_EQ(jeyson::get<int>(j["Two"]), 2);
    }

    // Move assignment reference from reference;
    {
        json j;
        j["One"] = 1;
        j["Two"] = 2;

        CHECK_EQ(jeyson::get<int>(j["One"]), 1);
        CHECK_EQ(jeyson::get<int>(j["Two"]), 2);

        auto ref = j["Two"];
        CHECK(ref);

        j["One"] = std::move(ref);

        CHECK_EQ(jeyson::get<int>(j["One"]), 2);
        CHECK_FALSE(ref);
    }

    // Assing to invalid JSON (non-initialized)
    {
        json j;
        j = nullptr;

        CHECK(is_null(j));

        j = true;

        CHECK(is_bool(j));
        CHECK_EQ(jeyson::get<bool>(j), true);
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
        REQUIRE_EQ(jeyson::get<bool>(j[1]), true);
        REQUIRE_EQ(jeyson::get<int>(j[2]), 42);
    }

    // Bad
    {
        std::string s {"[null"};

        REQUIRE_THROWS(json::parse(s));
    }

    {
        auto popts = doctest::getContextOptions();
        auto program = fs::path(pfs::utf8_decode_path(popts->binary_name.c_str()));
        auto program_dir = program.parent_path();

        auto j1 = json::parse(program_dir / pfs::utf8_decode_path("data/twitter.json"));
        auto j2 = json::parse(program_dir / pfs::utf8_decode_path("data/canada.json"));
        auto j3 = json::parse(program_dir / pfs::utf8_decode_path("data/citm_catalog.json"));

        REQUIRE(j1);
        REQUIRE(j2);
        REQUIRE(j3);

        auto code = j1["statuses"][0]["metadata"]["iso_language_code"];
        REQUIRE(code.is_string());
        REQUIRE_EQ(jeyson::get<std::string>(code), std::string{"ja"});
    }
}

template <typename Backend>
void run_algorithm_tests ()
{
    using json = jeyson::json<Backend>;
    using json_ref = jeyson::json_ref<Backend>;

    // Unitialized value
    {
        json j;
        int counter = 0;
        j.for_each([& counter] (json_ref) { counter++; });
        CHECK_EQ(counter, 0);
    }

    // Scalar
    {
        json j {42};
        int counter = 0;
        j.for_each([& counter] (json_ref) { counter++; });
        CHECK_EQ(counter, 0);
    }

    // Array
    {
        json j;
        j.push_back(json{nullptr});
        j.push_back(json{true});
        j.push_back(json{42});

        int counter = 0;
        j.for_each([& counter] (json_ref) { counter++; });
        CHECK_EQ(counter, 3);
    }

    // Array
    {
        json j;
        j.push_back(json{42});
        j.push_back(json{43});
        j.push_back(json{44});

        int counter = 42;
        j.for_each([& counter] (json_ref r) { CHECK_EQ(counter++, r.template get<int>()); });
        CHECK_EQ(counter, 45);
    }

    // Object
    {
        json j;
        j["0"] = 42;
        j["1"] = 43;
        j["2"] = 44;

        int counter = 42;
        j.for_each([& counter] (json_ref r) { CHECK_EQ(counter++, r.template get<int>()); });
        CHECK_EQ(counter, 45);
    }
}

template <typename Backend>
void run_serializer_tests ()
{
    using json = jeyson::json<Backend>;

    json j;

    j = nullptr;

    j["app"]["font"]["family"]    = "Roboto";
    j["app"]["font"]["pixelSize"] = 14;
    j["app"]["font"]["weight"]    = 50;
    j["app"]["font"]["italic"]    = true;

    j["messenger"]["font"]["family"]    = j["app"]["font"]["family"];
    j["messenger"]["font"]["pixelSize"] = j["app"]["font"]["pixelSize"];
    j["messenger"]["font"]["weight"]    = j["app"]["font"]["weight"];
    j["messenger"]["font"]["italic"]    = j["app"]["font"]["italic"];

    j["messenger"]["balloon"]["back"]["color"]["mine"]     = "#5d90c2";
    j["messenger"]["balloon"]["back"]["color"]["opponent"] = "#e0e0e0";
    j["messenger"]["balloon"]["fore"]["color"]["mine"]     = "#f8f8f8";
    j["messenger"]["balloon"]["fore"]["color"]["opponent"] = "#333333";

    CHECK_EQ(j["messenger"]["font"]["family"].template get<std::string>(), "Roboto");
    CHECK_EQ(j["messenger"]["font"]["pixelSize"].template get<int>(), 14);
    CHECK_EQ(j["messenger"]["font"]["weight"].template get<int>(), 50);
    CHECK_EQ(j["messenger"]["font"]["italic"].template get<bool>(), true);

    CHECK_EQ(j["messenger"]["balloon"]["back"]["color"]["mine"].template get<std::string>(), "#5d90c2");
    CHECK_EQ(j["messenger"]["balloon"]["back"]["color"]["opponent"].template get<std::string>(), "#e0e0e0");
    CHECK_EQ(j["messenger"]["balloon"]["fore"]["color"]["mine"].template get<std::string>(), "#f8f8f8");
    CHECK_EQ(j["messenger"]["balloon"]["fore"]["color"]["opponent"].template get<std::string>(), "#333333");

    auto text = to_string(j);

    CHECK_EQ(text, R"({"app":{"font":{"family":"Roboto","pixelSize":14,"weight":50,"italic":true}},"messenger":{"font":{"family":"Roboto","pixelSize":14,"weight":50,"italic":true},"balloon":{"back":{"color":{"mine":"#5d90c2","opponent":"#e0e0e0"}},"fore":{"color":{"mine":"#f8f8f8","opponent":"#333333"}}}}})");
    //fmt::print("{}\n", text);
}

TEST_CASE("JSON Jansson backend") {
    run_basic_tests<jeyson::backend::jansson>();
    run_decoder_tests();
    run_encoder_tests();
    run_assignment_tests<jeyson::backend::jansson>();
    run_access_tests<jeyson::backend::jansson>();
    run_parsing_tests<jeyson::backend::jansson>();
    run_algorithm_tests<jeyson::backend::jansson>();
    run_serializer_tests<jeyson::backend::jansson>();
}
