////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2019-2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2019.12.11 Initial version (pfs-json).
//      2022.02.07 Initial version (jeyson-lib).
////////////////////////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../doctest.h"
#include "pfs/jeyson/v1/parser.hpp"
#include <string>

TEST_CASE("is_whitespace") {
    using jeyson::v1::is_whitespace;

    CHECK(is_whitespace(' '));
    CHECK(is_whitespace('\t'));
    CHECK(is_whitespace('\r'));
    CHECK(is_whitespace('\n'));
    CHECK_FALSE(is_whitespace('x'));
}

TEST_CASE("is_digit") {
    using jeyson::v1::is_digit;

    CHECK(is_digit('0'));
    CHECK(is_digit('1'));
    CHECK(is_digit('2'));
    CHECK(is_digit('3'));
    CHECK(is_digit('4'));
    CHECK(is_digit('5'));
    CHECK(is_digit('6'));
    CHECK(is_digit('7'));
    CHECK(is_digit('8'));
    CHECK(is_digit('9'));
    CHECK_FALSE(is_digit('x'));
}

TEST_CASE("is_hexdigit") {
    using jeyson::v1::is_hexdigit;

    CHECK(is_hexdigit('0'));
    CHECK(is_hexdigit('1'));
    CHECK(is_hexdigit('2'));
    CHECK(is_hexdigit('3'));
    CHECK(is_hexdigit('4'));
    CHECK(is_hexdigit('5'));
    CHECK(is_hexdigit('6'));
    CHECK(is_hexdigit('7'));
    CHECK(is_hexdigit('8'));
    CHECK(is_hexdigit('9'));
    CHECK(is_hexdigit('a'));
    CHECK(is_hexdigit('b'));
    CHECK(is_hexdigit('c'));
    CHECK(is_hexdigit('d'));
    CHECK(is_hexdigit('e'));
    CHECK(is_hexdigit('f'));
    CHECK(is_hexdigit('A'));
    CHECK(is_hexdigit('B'));
    CHECK(is_hexdigit('C'));
    CHECK(is_hexdigit('D'));
    CHECK(is_hexdigit('E'));
    CHECK(is_hexdigit('F'));
    CHECK_FALSE(is_hexdigit('x'));
    CHECK_FALSE(is_hexdigit('X'));
}

TEST_CASE("to_digit") {
    using jeyson::v1::to_digit;

    CHECK(to_digit('0') == 0);
    CHECK(to_digit('1') == 1);
    CHECK(to_digit('8') == 8);
    CHECK(to_digit('9') == 9);
    CHECK(to_digit('a') < 0);

    CHECK(to_digit('a', 16) == 0xA);
    CHECK(to_digit('A', 16) == 0xA);
    CHECK(to_digit('f', 16) == 0xF);
    CHECK(to_digit('F', 16) == 0xF);
    CHECK(to_digit('X', 16) < 0);

    CHECK(to_digit('a', 36) == 10);
    CHECK(to_digit('A', 36) == 10);
    CHECK(to_digit('z', 36) == 35);
    CHECK(to_digit('Z', 36) == 35);
    CHECK(to_digit('@', 36) < 0);
}

struct test_advance_data
{
    std::string s; // char sequence
    bool r;        // result
    jeyson::v1::parse_policy_set parse_policy = jeyson::v1::strict_policy();
    std::error_code ec;

    test_advance_data(std::string const & a
            , bool b)
        : s(a), r(b)
    {}

    test_advance_data(std::string const & a
            , bool b
            , jeyson::v1::parse_policy_set const & c)
        : s(a), r(b), parse_policy(c)
    {}

    test_advance_data(std::string const & a
            , bool b
            , jeyson::v1::parse_policy_set const & c
            , jeyson::errc d)
        : s(a), r(b), parse_policy(c), ec(jeyson::make_error_code(d))
    {}
};

TEST_CASE("advance_null") {
    using jeyson::v1::advance_null;

    test_advance_data data[] = {
          { "n", false }
        , { "nu", false }
        , { "nul", false }
        , { "null", true }
        , { "null-", true }
        , { "NULL", false }
    };

    for (int i = 0, count = sizeof(data) / sizeof(data[0])
            ; i < count; i++) {
        auto pos  = data[i].s.begin();
        auto last = data[i].s.end();

        CHECK(advance_null(pos, last) == data[i].r);
    }
}

TEST_CASE("advance_true") {
    using jeyson::v1::advance_true;

    test_advance_data data[] = {
          { "t", false }
        , { "tr", false }
        , { "tru", false }
        , { "true", true }
        , { "true-", true }
        , { "TRUE", false }
    };

    for (int i = 0, count = sizeof(data) / sizeof(data[0])
            ; i < count; i++) {
        auto pos  = data[i].s.begin();
        auto last = data[i].s.end();

        CHECK(advance_true(pos, last) == data[i].r);
    }
}

TEST_CASE("advance_false") {
    using jeyson::v1::advance_false;

    test_advance_data data[] = {
          { "f", false }
        , { "fa", false }
        , { "fal", false }
        , { "fals", false }
        , { "false", true }
        , { "false-", true }
        , { "FALSE", false }
    };

    for (int i = 0, count = sizeof(data) / sizeof(data[0])
            ; i < count; i++) {
        auto pos  = data[i].s.begin();
        auto last = data[i].s.end();

        CHECK(advance_false(pos, last) == data[i].r);
    }
}

TEST_CASE("advance_encoded_char") {
    using jeyson::v1::advance_encoded_char;

    test_advance_data data[] = {
          { "a", false }
        , { "ab", false }
        , { "ABC", false }
        , { "ABCD", true }
        , { "0000", true }
        , { "99999", true }
        , { "ABCS", false }
    };

    for (int i = 0, count = sizeof(data) / sizeof(data[0])
            ; i < count; i++) {
        auto pos  = data[i].s.begin();
        auto last = data[i].s.end();
        int32_t ch;
        CHECK(advance_encoded_char(pos, last, ch) == data[i].r);
    }
}

TEST_CASE("advance_string") {
    using jeyson::v1::advance_string;

    test_advance_data data[] = {
          { "\"\"", true, jeyson::v1::strict_policy() }
        , { "''", true, jeyson::v1::json5_policy() }
        , { "\"simple string\"", true }

        , { "\"unquoted string"
            , false
            , jeyson::v1::strict_policy()
            , jeyson::errc::unbalanced_quote }

        , { "\"good escaped\\\" char\"", true }
        , { "\"good escaped\\\\ char\"", true }
        , { "\"good escaped\\/ char\"", true }
        , { "\"good escaped\\b char\"", true }
        , { "\"good escaped\\f char\"", true }
        , { "\"good escaped\\n char\"", true }
        , { "\"good escaped\\r char\"", true }
        , { "\"good escaped\\t char\"", true }
        , { "\"good escaped\\X char\"", true, jeyson::v1::relaxed_policy() }

        , { "\"bad escaped \\char\""
            , false
            , jeyson::v1::strict_policy()
            , jeyson::errc::bad_escaped_char }

        // Use encoded chars from ASCII table as output iterator will use
        // std::string::iterator to avoid 'SIGSEGV - Segmentation violation signal' (GCC)
        , { "\"good encoded \\u0020 char\"" , true }

        , { "\"bad encoded \\u0 char\""
            , false
            , jeyson::v1::strict_policy()
            , jeyson::errc::bad_encoded_char }
    };

    for (int i = 0, count = sizeof(data) / sizeof(data[0])
            ; i < count; i++) {
        auto pos  = data[i].s.begin();
        auto last = data[i].s.end();
        std::error_code ec;

        std::string s;

        CHECK(advance_string(pos
            , last
            , data[i].parse_policy
            , std::back_inserter(s)
            , ec) == data[i].r);
        CHECK(ec == data[i].ec);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Test advance_number
////////////////////////////////////////////////////////////////////////////////
struct Number
{
    enum { signed_integer, unsigned_integer, floating_point } type;
    union {
        intmax_t integer;
        uintmax_t uinteger;
        double real;
    };

    Number () : type(signed_integer), integer(0) {}

    Number (Number const & rhs) {
        switch (rhs.type) {
            case signed_integer: integer = rhs.integer;
            case unsigned_integer: uinteger = rhs.uinteger;
            case floating_point: real = rhs.real;
        }
    }

    Number (intmax_t n) : type(signed_integer), integer(n) {}
    Number (uintmax_t n) : type(unsigned_integer), uinteger(n) {}
    Number (double n) : type(floating_point), real(n) {}

    bool operator == (Number const & rhs) const
    {
        switch (type) {
            case signed_integer: return integer == rhs.integer;
            case unsigned_integer: return uinteger == rhs.uinteger;
            case floating_point: return real == rhs.real;
        }

        return false;
    }

    Number & operator = (intmax_t n)
    {
        integer = n;
        type = signed_integer;
        return *this;
    }

    Number & operator = (uintmax_t n)
    {
        uinteger = n;
        type = unsigned_integer;
        return *this;
    }

    Number & operator = (double n)
    {
        real = n;
        type = floating_point;
        return *this;
    }
};

template <typename T>
struct test_advance_number : test_advance_data
{
    T num;

    test_advance_number(T const & n
            , std::string const & a
            , bool b)
        : test_advance_data(a, b)
        , num(n)
    {}

    test_advance_number(T const & n
            , std::string const & a
            , bool b
            , jeyson::v1::parse_policy_set const & c)
        : test_advance_data(a, b, c)
        , num(n)
    {}

    test_advance_number(T const & n
            , std::string const & a
            , bool b
            , jeyson::v1::parse_policy_set const & c
            , jeyson::errc d)
        : test_advance_data(a, b, c, d)
        , num(n)
    {}
};

TEST_CASE("advance_number for integers") {
    using integer_type = int;
    using jeyson::v1::advance_number;
    using jeyson::v1::relaxed_policy;

    integer_type max_integer = std::numeric_limits<integer_type>::max();
    integer_type min_integer = std::numeric_limits<integer_type>::min();
    std::string max_integer_str = std::to_string(max_integer);
    std::string min_integer_str = std::to_string(min_integer);

    test_advance_number<integer_type> data[] = {
          {    0, "0"   , true }
        , {   -1, "-1"  , true }
        , {    1, "+1"  , false }
        , {    1, "+1"  , true, relaxed_policy() }
        , {    1, "1"   , true }
        , {  256, "256" , true }
        , { -256, "-256" , true }
        , {  max_integer, max_integer_str, true }
        , {  min_integer, min_integer_str, true }
    };

    for (int i = 0, count = sizeof(data) / sizeof(data[0])
            ; i < count; i++) {
        auto pos  = data[i].s.begin();
        auto last = data[i].s.end();
        std::error_code ec;

        integer_type num;
        auto ok = advance_number(pos, last, data[i].parse_policy, & num, ec);

        CHECK(ok == data[i].r);

        if (ok)
            CHECK(num == data[i].num);
    }
}

TEST_CASE("advance_number for custom number type") {
    using jeyson::v1::advance_number;
    using jeyson::v1::relaxed_policy;

    test_advance_number<Number> data[] = {
          { Number{intmax_t(0)}, "0", true }
        , { Number{intmax_t(-1)}, "-1", true }
        , { Number{intmax_t(1)}, "+1", false }
        , { Number{intmax_t(1)}, "+1", true, relaxed_policy() }
        , { Number{uintmax_t(1)}, "1", true }
    };

    for (int i = 0, count = sizeof(data) / sizeof(data[0])
            ; i < count; i++) {
        auto pos  = data[i].s.begin();
        auto last = data[i].s.end();
        std::error_code ec;

        Number num;
        auto ok = advance_number(pos, last, data[i].parse_policy, & num, ec);

        CHECK(ok == data[i].r);

        if (ok)
            CHECK(num == data[i].num);
    }
}

TEST_CASE("parse_array of booleans") {
    using jeyson::v1::parse_array;
    using jeyson::v1::strict_policy;

    auto arr_str = std::string{"[true, true, false, true, false]"};
    std::vector<bool> arr;
    std::error_code ec;

    auto first = arr_str.begin();
    auto last = arr_str.end();
    auto pos = parse_array(first, last, strict_policy(), arr, ec);
    REQUIRE(ec == std::error_code{});
    REQUIRE(pos == last);
    REQUIRE(arr.size() == 5);
    CHECK(arr[0] == true);
    CHECK(arr[1] == true);
    CHECK(arr[2] == false);
    CHECK(arr[3] == true);
    CHECK(arr[4] == false);
}

TEST_CASE("parse_array of integers") {
    using jeyson::v1::parse_array;
    using jeyson::v1::strict_policy;

    auto arr_str = std::string{"[1, 2, 3, 4, 5]"};
    std::vector<int> arr;
    std::error_code ec;

    auto first = arr_str.begin();
    auto last = arr_str.end();
    auto pos = parse_array(first, last, strict_policy(), arr, ec);
    REQUIRE(ec == std::error_code{});
    REQUIRE(pos == last);
    REQUIRE(arr.size() == 5);
    CHECK(arr[0] == 1);
    CHECK(arr[1] == 2);
    CHECK(arr[2] == 3);
    CHECK(arr[3] == 4);
    CHECK(arr[4] == 5);
}

TEST_CASE("parse_array of floating points") {
    using jeyson::v1::parse_array;
    using jeyson::v1::strict_policy;

    auto arr_str = std::string{"[0.1, 0.2, 0.3, 0.4, 0.5]"};
    std::vector<double> arr;
    std::error_code ec;

    auto first = arr_str.begin();
    auto last = arr_str.end();
    auto pos = parse_array(first, last, strict_policy(), arr, ec);
    REQUIRE(ec == std::error_code{});
    REQUIRE(pos == last);
    REQUIRE(arr.size() == 5);
    CHECK(arr[0] == std::stod("0.1"));
    CHECK(arr[1] == std::stod("0.2"));
    CHECK(arr[2] == std::stod("0.3"));
    CHECK(arr[3] == std::stod("0.4"));
    CHECK(arr[4] == std::stod("0.5"));
}

TEST_CASE("parse_array of strings") {
    using jeyson::v1::parse_array;
    using jeyson::v1::strict_policy;

    auto arr_str = std::string{"[\"one\", \"two\", \"three\"]"};
    std::vector<std::string> arr;
    std::error_code ec;

    auto first = arr_str.begin();
    auto last = arr_str.end();
    auto pos = parse_array(first, last, strict_policy(), arr, ec);
    REQUIRE(ec == std::error_code{});
    REQUIRE(pos == last);
    REQUIRE(arr.size() == 3);
    CHECK(arr[0] == "one");
    CHECK(arr[1] == "two");
    CHECK(arr[2] == "three");
}

TEST_CASE("parse_object of booleans") {
    using jeyson::v1::parse_object;
    using jeyson::v1::strict_policy;

    auto obj_str = std::string{"{\"one\": true, \"two\": false, \"three\": true}"};
    std::map<std::string, bool> obj;
    std::error_code ec;

    auto first = obj_str.begin();
    auto last = obj_str.end();
    auto pos = parse_object(first, last, strict_policy(), obj, ec);
    REQUIRE(ec == std::error_code{});
    REQUIRE(pos == last);
    REQUIRE(obj.size() == 3);
    CHECK(obj["one"] == true);
    CHECK(obj["two"] == false);
    CHECK(obj["three"] == true);
}

TEST_CASE("parse_object of integers") {
    using jeyson::v1::parse_object;
    using jeyson::v1::strict_policy;

    auto obj_str = std::string{"{\"one\": 1, \"two\": 2, \"three\": 3}"};
    std::map<std::string, int> obj;
    std::error_code ec;

    auto first = obj_str.begin();
    auto last = obj_str.end();
    auto pos = parse_object(first, last, strict_policy(), obj, ec);
    REQUIRE(ec == std::error_code{});
    REQUIRE(pos == last);
    REQUIRE(obj.size() == 3);
    CHECK(obj["one"] == 1);
    CHECK(obj["two"] == 2);
    CHECK(obj["three"] == 3);
}

TEST_CASE("parse_object of floating point") {
    using jeyson::v1::parse_object;
    using jeyson::v1::strict_policy;

    auto obj_str = std::string{"{\"one\": 0.1, \"two\": 0.2, \"three\": 0.3}"};
    std::map<std::string, double> obj;
    std::error_code ec;

    auto first = obj_str.begin();
    auto last = obj_str.end();
    auto pos = parse_object(first, last, strict_policy(), obj, ec);
    REQUIRE(ec == std::error_code{});
    REQUIRE(pos == last);
    REQUIRE(obj.size() == 3);
    CHECK(obj["one"] == std::stod("0.1"));
    CHECK(obj["two"] == std::stod("0.2"));
    CHECK(obj["three"] == std::stod("0.3"));
}

TEST_CASE("parse_object of strings") {
    using jeyson::v1::parse_object;
    using jeyson::v1::strict_policy;

    auto obj_str = std::string{"{\"one\": \"one\", \"two\": \"two\", \"three\": \"three\"}"};
    std::map<std::string, std::string> obj;
    std::error_code ec;

    auto first = obj_str.begin();
    auto last = obj_str.end();
    auto pos = parse_object(first, last, strict_policy(), obj, ec);
    REQUIRE(ec == std::error_code{});
    REQUIRE(pos == last);
    REQUIRE(obj.size() == 3);
    CHECK(obj["one"] == "one");
    CHECK(obj["two"] == "two");
    CHECK(obj["three"] == "three");
}

