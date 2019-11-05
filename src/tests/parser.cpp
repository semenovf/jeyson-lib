#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/json/parser.hpp"
#include <cstring>

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

TEST_CASE("advance_number") {
    using pfs::json::advance_number;

    static char const * valid_numbers[] = {
          "0"
        , "1"
        , "+0"
        , "+1"
    };

    for (int i = 0, count = sizeof(valid_numbers) / sizeof(valid_numbers[0])
            ; i < count; i++) {
        auto pos  = valid_numbers[i];
        auto last = valid_numbers[i] + std::strlen(valid_numbers[i]);

        CHECK(advance_number(pos, last));
    }
}
