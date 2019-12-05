#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/json/json.hpp"
#include <limits>

TEST_CASE("JSON constructors") {
    using json_value = pfs::json::value<>;

    {
        json_value v;
        CHECK(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{true};
        CHECK(v.is_boolean());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{false};
        CHECK(v.is_boolean());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{std::numeric_limits<int>::min()};
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{std::numeric_limits<int>::max()};
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{std::numeric_limits<unsigned int>::min()};
        CHECK(v.is_uinteger());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{std::numeric_limits<unsigned int>::max()};
        CHECK(v.is_uinteger());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{std::numeric_limits<float>::min()};
        CHECK(v.is_real());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{std::numeric_limits<float>::max()};
        CHECK(v.is_real());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{std::numeric_limits<double>::min()};
        CHECK(v.is_real());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{std::numeric_limits<double>::max()};
        CHECK(v.is_real());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{0};
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{-1};
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{1};
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v{"hello"};
        CHECK(v.is_string());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }
}
