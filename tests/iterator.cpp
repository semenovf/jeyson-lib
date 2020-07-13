#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/json/json.hpp"
#include <limits>

TEST_CASE("Iterators") {
    using json_value = pfs::json::value<>;
//     using type_enum = pfs::json::type_enum;

    // Scalar values
    {
        std::vector<json_value> values {json_value{}
                , json_value{true}
                , json_value{std::numeric_limits<int>::min()}
                , json_value{"hello"} };

        for (auto & v: values) {
            auto first = std::begin(v);
            auto last = std::end(v);
            CHECK(first != last);
            CHECK(++first == last);
        }

        auto first = values[0].begin();
        CHECK(first->is_null());

        first = values[1].begin();
        CHECK(first->is_boolean());
        CHECK(first->get<bool>() == true);
    }

    {
        // Array
        // FIXME
//         json_value v(type_enum::array);
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
        // FIXME
//         json_value v(type_enum::object);
//         CHECK(v.empty());
//         CHECK(v.size() == 0);
//         CHECK(v.max_size() == json_value::object_type().max_size());
    }}

