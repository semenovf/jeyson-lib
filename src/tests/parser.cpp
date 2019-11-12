#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/json/parser.hpp"
#include <string>

TEST_CASE("is_whitespace") {
    using pfs::json::is_whitespace;

    CHECK(is_whitespace(' '));
    CHECK(is_whitespace('\t'));
    CHECK(is_whitespace('\r'));
    CHECK(is_whitespace('\n'));
    CHECK_FALSE(is_whitespace('x'));
}

TEST_CASE("is_digit") {
    using pfs::json::is_digit;

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
    using pfs::json::is_hexdigit;

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
    using pfs::json::to_digit;

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
    pfs::json::parse_policy_set parse_policy = pfs::json::strict_policy();
    pfs::json::error_code ec;

    test_advance_data(std::string const & a
            , bool b)
        : s(a), r(b)
    {}

    test_advance_data(std::string const & a
            , bool b
            , pfs::json::parse_policy_set const & c)
        : s(a), r(b), parse_policy(c)
    {}

    test_advance_data(std::string const & a
            , bool b
            , pfs::json::parse_policy_set const & c
            , pfs::json::errc d)
        : s(a), r(b), parse_policy(c), ec(pfs::json::make_error_code(d))
    {}
};

TEST_CASE("advance_null") {
    using pfs::json::advance_null;

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
    using pfs::json::advance_true;

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
    using pfs::json::advance_false;

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
    using pfs::json::advance_encoded_char;

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

        CHECK(advance_encoded_char(pos, last) == data[i].r);
    }
}

TEST_CASE("advance_string") {
    using pfs::json::advance_string;

    pfs::json::string_context<std::string::iterator, std::string::iterator> * ctx = nullptr;

    test_advance_data data[] = {
          { "\"\"", true, pfs::json::strict_policy() }
        , { "''", true, pfs::json::json5_policy() }
        , { "\"simple string\"", true }

        , { "\"unquoted string"
                , false
                , pfs::json::strict_policy()
                , pfs::json::errc::unquoted_string }

        , { "\"good escaped\\\" char\"", true }
        , { "\"good escaped\\\\ char\"", true }
        , { "\"good escaped\\/ char\"", true }
        , { "\"good escaped\\b char\"", true }
        , { "\"good escaped\\f char\"", true }
        , { "\"good escaped\\n char\"", true }
        , { "\"good escaped\\r char\"", true }
        , { "\"good escaped\\t char\"", true }
        , { "\"good escaped\\X char\"", true, pfs::json::relaxed_policy() }

        , { "\"bad escaped \\char\""
                , false
                , pfs::json::strict_policy()
                , pfs::json::errc::bad_escaped_char }

        , { "\"good encoded \\uFFFF char\"" , true }

        , { "\"bad encoded \\u0 char\""
                , false
                , pfs::json::strict_policy()
                , pfs::json::errc::bad_encoded_char }
    };

    for (int i = 0, count = sizeof(data) / sizeof(data[0])
            ; i < count; i++) {
        auto pos  = data[i].s.begin();
        auto last = data[i].s.end();
        pfs::json::error_code ec;

        CHECK(advance_string(pos, last, data[i].parse_policy, ctx, ec) == data[i].r);
        CHECK(ec == data[i].ec);
    }
}

TEST_CASE("advance_number") {
    using pfs::json::advance_number;

//     struct test_data {}
    static char const * valid_numbers[] = {
          "0"
        , "1"
//         , "+0"
//         , "+1"
    };

    for (int i = 0, count = sizeof(valid_numbers) / sizeof(valid_numbers[0])
            ; i < count; i++) {
        auto pos  = valid_numbers[i];
        auto last = valid_numbers[i] + std::strlen(valid_numbers[i]);

        CHECK(advance_number(pos, last));
    }
}
