#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/json/json.hpp"
#include <limits>

TEST_CASE("Constructors") {
    using json_value = pfs::json::value<>;
    using type_enum = pfs::json::type_enum;

    {
        json_value v;
        CHECK(v.type() == type_enum::null);
        CHECK(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(true);
        CHECK(v.type() == type_enum::boolean);
        CHECK(v.is_boolean());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(false);
        CHECK(v.type() == type_enum::boolean);
        CHECK(v.is_boolean());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(std::numeric_limits<int>::min());
        CHECK(v.type() == type_enum::integer);
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(std::numeric_limits<int>::max());
        CHECK(v.type() == type_enum::integer);
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(std::numeric_limits<unsigned int>::min());
        CHECK(v.type() == type_enum::uinteger);
        CHECK(v.is_uinteger());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(std::numeric_limits<unsigned int>::max());
        CHECK(v.type() == type_enum::uinteger);
        CHECK(v.is_uinteger());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(std::numeric_limits<float>::min());
        CHECK(v.type() == type_enum::real);
        CHECK(v.is_real());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(std::numeric_limits<float>::max());
        CHECK(v.type() == type_enum::real);
        CHECK(v.is_real());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(std::numeric_limits<double>::min());
        CHECK(v.type() == type_enum::real);
        CHECK(v.is_real());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(std::numeric_limits<double>::max());
        CHECK(v.type() == type_enum::real);
        CHECK(v.is_real());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(0);
        CHECK(v.type() == type_enum::integer);
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(-1);
        CHECK(v.type() == type_enum::integer);
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(1);
        CHECK(v.type() == type_enum::integer);
        CHECK(v.is_integer());
        CHECK(v.is_number());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v("hello");
        CHECK(v.type() == type_enum::string);
        CHECK(v.is_string());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_array());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(type_enum::array);
        CHECK(v.type() == type_enum::array);

        CHECK(v.is_array());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_object());
    }

    {
        json_value v(type_enum::object);
        CHECK(v.type() == type_enum::object);

        CHECK(v.is_object());
        CHECK_FALSE(v.is_null());
        CHECK_FALSE(v.is_boolean());
        CHECK_FALSE(v.is_number());
        CHECK_FALSE(v.is_string());
        CHECK_FALSE(v.is_array());
    }
}

TEST_CASE("Cast") {
    using json_value = pfs::json::value<>;
    using std::to_string;
    using pfs::json::to_string;

    {
        for (auto sample_value: { false, true}) {
            json_value v(sample_value);
            CHECK(v.get<bool>() == sample_value);
            CHECK(v.get<char>() == static_cast<char>(sample_value));
            CHECK(v.get<signed char>() == static_cast<signed char>(sample_value));
            CHECK(v.get<unsigned char>() == static_cast<unsigned char>(sample_value));
            CHECK(v.get<short int>() == static_cast<short int>(sample_value));
            CHECK(v.get<unsigned short int>() == static_cast<unsigned short int>(sample_value));
            CHECK(v.get<int>() == static_cast<int>(sample_value));
            CHECK(v.get<unsigned int>() == static_cast<unsigned int>(sample_value));
            CHECK(v.get<long int>() == static_cast<long int>(sample_value));
            CHECK(v.get<unsigned long int>() == static_cast<unsigned long int>(sample_value));
            CHECK(v.get<long long int>() == static_cast<long long int>(sample_value));
            CHECK(v.get<unsigned long long int>() == static_cast<unsigned long long int>(sample_value));
            CHECK(v.get<float>() == static_cast<float>(sample_value));
            CHECK(v.get<double>() == static_cast<double>(sample_value));
            CHECK(v.get<json_value::string_type>() == (sample_value ? "true" : "false"));
        }
    }

    {
        // Signed integer
        auto sample_value = std::numeric_limits<int>::min();
        json_value v(sample_value);
        CHECK(v.get<bool>() == static_cast<bool>(sample_value));
        CHECK(v.get<char>() == static_cast<char>(sample_value));
        CHECK(v.get<signed char>() == static_cast<signed char>(sample_value));
        CHECK(v.get<unsigned char>() == static_cast<unsigned char>(sample_value));
        CHECK(v.get<short int>() == static_cast<short int>(sample_value));
        CHECK(v.get<unsigned short int>() == static_cast<unsigned short int>(sample_value));
        CHECK(v.get<int>() == sample_value);
        CHECK(v.get<unsigned int>() == static_cast<unsigned int>(sample_value));
        CHECK(v.get<long int>() == static_cast<long int>(sample_value));
        CHECK(v.get<unsigned long int>() == static_cast<unsigned long int>(sample_value));
        CHECK(v.get<long long int>() == static_cast<long long int>(sample_value));
        CHECK(v.get<unsigned long long int>() == static_cast<unsigned long long int>(sample_value));
        CHECK(v.get<float>() == static_cast<float>(sample_value));
        CHECK(v.get<double>() == static_cast<double>(sample_value));
        CHECK(v.get<json_value::string_type>() == to_string(sample_value));
    }

    {
        // Unsigned integer
        auto sample_value = std::numeric_limits<unsigned int>::min();
        json_value v(sample_value);
        CHECK(v.get<bool>() == static_cast<bool>(sample_value));
        CHECK(v.get<char>() == static_cast<char>(sample_value));
        CHECK(v.get<signed char>() == static_cast<signed char>(sample_value));
        CHECK(v.get<unsigned char>() == static_cast<unsigned char>(sample_value));
        CHECK(v.get<short int>() == static_cast<short int>(sample_value));
        CHECK(v.get<unsigned short int>() == static_cast<unsigned short int>(sample_value));
        CHECK(v.get<int>() == static_cast<int>(sample_value));
        CHECK(v.get<unsigned int>() == sample_value);
        CHECK(v.get<long int>() == static_cast<long int>(sample_value));
        CHECK(v.get<unsigned long int>() == static_cast<unsigned long int>(sample_value));
        CHECK(v.get<long long int>() == static_cast<long long int>(sample_value));
        CHECK(v.get<unsigned long long int>() == static_cast<unsigned long long int>(sample_value));
        CHECK(v.get<float>() == static_cast<float>(sample_value));
        CHECK(v.get<double>() == static_cast<double>(sample_value));
        CHECK(v.get<json_value::string_type>() == to_string(sample_value));
    }

    {
        // Floating point
        auto sample_value = double(3.14159);
        json_value v(sample_value);
        CHECK(v.get<bool>() == static_cast<bool>(sample_value));
        CHECK(v.get<char>() == static_cast<char>(sample_value));
        CHECK(v.get<signed char>() == static_cast<signed char>(sample_value));
        CHECK(v.get<unsigned char>() == static_cast<unsigned char>(sample_value));
        CHECK(v.get<short int>() == static_cast<short int>(sample_value));
        CHECK(v.get<unsigned short int>() == static_cast<unsigned short int>(sample_value));
        CHECK(v.get<int>() == static_cast<int>(sample_value));
        CHECK(v.get<unsigned int>() == static_cast<int>(sample_value));
        CHECK(v.get<long int>() == static_cast<long int>(sample_value));
        CHECK(v.get<unsigned long int>() == static_cast<unsigned long int>(sample_value));
        CHECK(v.get<long long int>() == static_cast<long long int>(sample_value));
        CHECK(v.get<unsigned long long int>() == static_cast<unsigned long long int>(sample_value));
        CHECK(v.get<float>() == static_cast<float>(sample_value));
        CHECK(v.get<double>() == sample_value);
        CHECK(v.get<json_value::string_type>() == to_string(sample_value));
    }

    {
        // String
        auto sample_value = json_value::string_type("hello");
        json_value v(sample_value);
        CHECK_THROWS_AS(v.get<bool>(), std::system_error);
        CHECK_THROWS_AS(v.get<char>(), std::system_error);
        CHECK_THROWS_AS(v.get<signed char>(), std::system_error);
        CHECK_THROWS_AS(v.get<unsigned char>(), std::system_error);
        CHECK_THROWS_AS(v.get<short int>(), std::system_error);
        CHECK_THROWS_AS(v.get<unsigned short int>(), std::system_error);
        CHECK_THROWS_AS(v.get<int>(), std::system_error);
        CHECK_THROWS_AS(v.get<unsigned int>(), std::system_error);
        CHECK_THROWS_AS(v.get<long int>(), std::system_error);
        CHECK_THROWS_AS(v.get<unsigned long int>(), std::system_error);
        CHECK_THROWS_AS(v.get<long long int>(), std::system_error);
        CHECK_THROWS_AS(v.get<unsigned long long int>(), std::system_error);
        CHECK_THROWS_AS(v.get<float>(), std::system_error);
        CHECK_THROWS_AS(v.get<double>(), std::system_error);
        CHECK(v.get<json_value::string_type>() == sample_value);
    }

    {
        // TODO Array
    }

    {
        // TODO Object
    }
}

TEST_CASE("Capacity") {
    using json_value = pfs::json::value<>;
    using type_enum = pfs::json::type_enum;

    {   // Null
        json_value v;

        CHECK(v.empty());
        CHECK(v.size() == 0);
        CHECK(v.max_size() == 0);
    }

    {
        // Boolean
        json_value v(true);
        CHECK_FALSE(v.empty());
        CHECK(v.size() == 1);
        CHECK(v.max_size() == 1);
    }

    {
        // Number
        json_value v(std::numeric_limits<int>::min());
        CHECK_FALSE(v.empty());
        CHECK(v.size() == 1);
        CHECK(v.max_size() == 1);
    }

    {
        // String
        json_value v("hello");
        CHECK_FALSE(v.empty());
        CHECK(v.size() == 1);
        CHECK(v.max_size() == 1);
    }

    {
        // Array
        json_value v(type_enum::array);
        CHECK(v.empty());
        CHECK(v.size() == 0);
        CHECK(v.max_size() == json_value::array_type().max_size());
    }

    {
        // Object
        json_value v(type_enum::object);
        CHECK(v.empty());
        CHECK(v.size() == 0);
        CHECK(v.max_size() == json_value::object_type().max_size());
    }
}

TEST_CASE("Modifiers") {
    using json_value = pfs::json::value<>;
    using type_enum = pfs::json::type_enum;

    {   // Null
        json_value v;
        CHECK(v.is_null());
        v.clear();
        CHECK(v.is_null());
    }

    {
        // Boolean
        json_value v(true);
        CHECK(v.get<bool>() == true);
        v.clear();
        CHECK(v.get<bool>() == false);
    }

    {
        // Number
        json_value v(std::numeric_limits<int>::min());
        CHECK(v.get<int>() == std::numeric_limits<int>::min());
        v.clear();
        CHECK(v.get<int>() == int(0));
    }

    {
        // String
        json_value v("hello");
        CHECK_FALSE(v.get<std::string>().empty());
        v.clear();
        CHECK(v.get<std::string>().empty());
    }

    {
        // Array
        json_value v(type_enum::array);
//         v.push_back(std::move(nullptr));
//         v += nullptr;

//         int x = 10;
//         v.push_back(x);

//         json_value arr(type_enum::array);
//         v.push_back(std::move(arr));
//
//         json_value obj(type_enum::object);
//         v.push_back(std::move(obj));

//         CHECK(v.empty());
//         CHECK(v.size() == 0);
//         CHECK(v.max_size() == json_value::array_type().max_size());
    }

    {
        // Object
        json_value v(type_enum::object);
//         CHECK(v.empty());
//         CHECK(v.size() == 0);
//         CHECK(v.max_size() == json_value::object_type().max_size());
    }
}

TEST_CASE("Iterators") {
    using json_value = pfs::json::value<>;
    using type_enum = pfs::json::type_enum;
}
