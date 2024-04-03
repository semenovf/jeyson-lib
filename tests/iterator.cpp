////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2020-2022 Vladislav Trifochkin
//
// This file is part of `jeyson-lib`.
//
// Changelog:
//      2020.04.15 Initial version (pfs-json).
//      2022.02.07 Initial version (jeyson-lib).
//      2022.09.26 Refactored.
//      2024.03.31 Refactored based on tests from nlohmann/json repository.
////////////////////////////////////////////////////////////////////////////////
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "pfs/jeyson/json.hpp"
#include "pfs/jeyson/backend/jansson.hpp"

template <typename Backend>
void run_iterator_tests ()
{
    using json = jeyson::json<Backend>;

    SUBCASE("basic behavior") {
        SUBCASE("uninitialized") {
            json j;

            typename json::iterator it;
            typename json::const_iterator cit;
        }

        SUBCASE("boolean") {
            json j {true};
            json const j_const(j);

            SUBCASE("json + begin/end") {
                typename json::iterator it = j.begin();
                CHECK(it != j.end());
                CHECK(*it == j);

                it++;
                CHECK(it != j.begin());
                CHECK(it == j.end());

                it--;
                CHECK(it == j.begin());
                CHECK(it != j.end());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.begin());
                CHECK(it == j.end());

                --it;
                CHECK(it == j.begin());
                CHECK(it != j.end());
                CHECK(*it == j);
            }

            SUBCASE("const json + begin/end") {
                typename json::const_iterator it = j_const.begin();
                CHECK(it != j_const.end());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.begin());
                CHECK(it == j_const.end());

                it--;
                CHECK(it == j_const.begin());
                CHECK(it != j_const.end());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.begin());
                CHECK(it == j_const.end());

                --it;
                CHECK(it == j_const.begin());
                CHECK(it != j_const.end());
                CHECK(*it == j_const);
            }

            SUBCASE("json + cbegin/cend") {
                typename json::const_iterator it = j.cbegin();
                CHECK(it != j.cend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.cbegin());
                CHECK(it == j.cend());

                it--;
                CHECK(it == j.cbegin());
                CHECK(it != j.cend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.cbegin());
                CHECK(it == j.cend());

                --it;
                CHECK(it == j.cbegin());
                CHECK(it != j.cend());
                CHECK(*it == j);
            }

            SUBCASE("const json + cbegin/cend") {
                typename json::const_iterator it = j_const.cbegin();
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.cbegin());
                CHECK(it == j_const.cend());

                it--;
                CHECK(it == j_const.cbegin());
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.cbegin());
                CHECK(it == j_const.cend());

                --it;
                CHECK(it == j_const.cbegin());
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);
            }

            SUBCASE("json + rbegin/rend") {
                typename json::reverse_iterator it = j.rbegin();
                CHECK(it != j.rend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.rbegin());
                CHECK(it == j.rend());

                it--;
                CHECK(it == j.rbegin());
                CHECK(it != j.rend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.rbegin());
                CHECK(it == j.rend());

                --it;
                CHECK(it == j.rbegin());
                CHECK(it != j.rend());
                CHECK(*it == j);
            }

            SUBCASE("json + crbegin/crend") {
                typename json::const_reverse_iterator it = j.crbegin();
                CHECK(it != j.crend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.crbegin());
                CHECK(it == j.crend());

                it--;
                CHECK(it == j.crbegin());
                CHECK(it != j.crend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.crbegin());
                CHECK(it == j.crend());

                --it;
                CHECK(it == j.crbegin());
                CHECK(it != j.crend());
                CHECK(*it == j);
            }

            SUBCASE("const json + crbegin/crend") {
                typename json::const_reverse_iterator it = j_const.crbegin();
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.crbegin());
                CHECK(it == j_const.crend());

                it--;
                CHECK(it == j_const.crbegin());
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.crbegin());
                CHECK(it == j_const.crend());

                --it;
                CHECK(it == j_const.crbegin());
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);
            }

            SUBCASE("additional tests") {
                SUBCASE("!(begin != begin)") {
                    CHECK(!(j.begin() != j.begin()));
                }

                SUBCASE("!(end != end)") {
                    CHECK(!(j.end() != j.end()));
                }
            }

            SUBCASE("key/value") {
                auto it = j.begin();
                auto cit = j_const.cbegin();
                CHECK_THROWS_WITH_AS(it.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(it.value() == json(true));
                CHECK_THROWS_WITH_AS(cit.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(cit.value() == json(true));

                auto rit = j.rend();
                auto crit = j.crend();

                CHECK_THROWS_WITH_AS(rit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(rit.base().value() == json(true));
                CHECK_THROWS_WITH_AS(crit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(crit.base().value() == json(true));
            }
        }

        SUBCASE("string") {
            json j {"hello world"};
            json const j_const(j);

            SUBCASE("json + begin/end") {
                typename json::iterator it = j.begin();
                CHECK(it != j.end());
                CHECK(*it == j);

                it++;
                CHECK(it != j.begin());
                CHECK(it == j.end());

                it--;
                CHECK(it == j.begin());
                CHECK(it != j.end());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.begin());
                CHECK(it == j.end());

                --it;
                CHECK(it == j.begin());
                CHECK(it != j.end());
                CHECK(*it == j);
            }

            SUBCASE("const json + begin/end") {
                typename json::const_iterator it = j_const.begin();
                CHECK(it != j_const.end());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.begin());
                CHECK(it == j_const.end());

                it--;
                CHECK(it == j_const.begin());
                CHECK(it != j_const.end());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.begin());
                CHECK(it == j_const.end());

                --it;
                CHECK(it == j_const.begin());
                CHECK(it != j_const.end());
                CHECK(*it == j_const);
            }

            SUBCASE("json + cbegin/cend") {
                typename json::const_iterator it = j.cbegin();
                CHECK(it != j.cend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.cbegin());
                CHECK(it == j.cend());

                it--;
                CHECK(it == j.cbegin());
                CHECK(it != j.cend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.cbegin());
                CHECK(it == j.cend());

                --it;
                CHECK(it == j.cbegin());
                CHECK(it != j.cend());
                CHECK(*it == j);
            }

            SUBCASE("const json + cbegin/cend") {
                typename json::const_iterator it = j_const.cbegin();
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.cbegin());
                CHECK(it == j_const.cend());

                it--;
                CHECK(it == j_const.cbegin());
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.cbegin());
                CHECK(it == j_const.cend());

                --it;
                CHECK(it == j_const.cbegin());
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);
            }

            SUBCASE("json + rbegin/rend") {
                typename json::reverse_iterator it = j.rbegin();
                CHECK(it != j.rend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.rbegin());
                CHECK(it == j.rend());

                it--;
                CHECK(it == j.rbegin());
                CHECK(it != j.rend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.rbegin());
                CHECK(it == j.rend());

                --it;
                CHECK(it == j.rbegin());
                CHECK(it != j.rend());
                CHECK(*it == j);
            }

            SUBCASE("json + crbegin/crend") {
                typename json::const_reverse_iterator it = j.crbegin();
                CHECK(it != j.crend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.crbegin());
                CHECK(it == j.crend());

                it--;
                CHECK(it == j.crbegin());
                CHECK(it != j.crend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.crbegin());
                CHECK(it == j.crend());

                --it;
                CHECK(it == j.crbegin());
                CHECK(it != j.crend());
                CHECK(*it == j);
            }

            SUBCASE("const json + crbegin/crend") {
                typename json::const_reverse_iterator it = j_const.crbegin();
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.crbegin());
                CHECK(it == j_const.crend());

                it--;
                CHECK(it == j_const.crbegin());
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.crbegin());
                CHECK(it == j_const.crend());

                --it;
                CHECK(it == j_const.crbegin());
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);
            }

            SUBCASE("key/value") {
                auto it = j.begin();
                auto cit = j_const.cbegin();
                CHECK_THROWS_WITH_AS(it.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(it.value() == json("hello world"));
                CHECK_THROWS_WITH_AS(cit.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(cit.value() == json("hello world"));

                auto rit = j.rend();
                auto crit = j.crend();

                CHECK_THROWS_WITH_AS(rit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(rit.base().value() == json("hello world"));
                CHECK_THROWS_WITH_AS(crit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(crit.base().value() == json("hello world"));
            }
        }

        SUBCASE("array") {
            json j;
            j.push_back(1);
            j.push_back(2);
            j.push_back(3);

            json const j_const(j);

            SUBCASE("json + begin/end") {
                typename json::iterator it_begin = j.begin();
                typename json::iterator it_end = j.end();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j[0]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[1]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[2]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("const json + begin/end") {
                typename json::const_iterator it_begin = j_const.begin();
                typename json::const_iterator it_end = j_const.end();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j_const[0]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j_const[1]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j_const[2]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("json + cbegin/cend") {
                typename json::const_iterator it_begin = j.cbegin();
                typename json::const_iterator it_end = j.cend();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j[0]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[1]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[2]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("const json + cbegin/cend") {
                typename json::const_iterator it_begin = j_const.cbegin();
                typename json::const_iterator it_end = j_const.cend();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j[0]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[1]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[2]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("json + rbegin/rend") {
                typename json::reverse_iterator it_begin = j.rbegin();
                typename json::reverse_iterator it_end = j.rend();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j[2]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[1]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[0]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("json + crbegin/crend") {
                typename json::const_reverse_iterator it_begin = j.crbegin();
                typename json::const_reverse_iterator it_end = j.crend();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j[2]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[1]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[0]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("const json + crbegin/crend") {
                typename json::const_reverse_iterator it_begin = j_const.crbegin();
                typename json::const_reverse_iterator it_end = j_const.crend();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j[2]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[1]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j[0]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("key/value") {
                auto it = j.begin();
                auto cit = j_const.cbegin();
                CHECK_THROWS_WITH_AS(it.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(it.value() == json(1));
                CHECK_THROWS_WITH_AS(cit.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(cit.value() == json(1));
            }
        }

        SUBCASE("object") {
            json j;

            j.insert("A", 1);
            j.insert("B", 2);
            j.insert("C", 3);

            json const j_const(j);

            SUBCASE("json + begin/end") {
                typename json::iterator it_begin = j.begin();
                typename json::iterator it_end = j.end();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j["A"]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j["B"]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j["C"]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("const json + begin/end") {
                typename json::const_iterator it_begin = j_const.begin();
                typename json::const_iterator it_end = j_const.end();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j_const["A"]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j_const["B"]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j_const["C"]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("json + cbegin/cend") {
                typename json::const_iterator it_begin = j.cbegin();
                typename json::const_iterator it_end = j.cend();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j["A"]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j["B"]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j["C"]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("const json + cbegin/cend") {
                typename json::const_iterator it_begin = j_const.cbegin();
                typename json::const_iterator it_end = j_const.cend();

                auto it = it_begin;
                CHECK(it != it_end);
                CHECK(*it == j_const["A"]);

                it++;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j_const["B"]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it != it_end);
                CHECK(*it == j_const["C"]);

                ++it;
                CHECK(it != it_begin);
                CHECK(it == it_end);
            }

            SUBCASE("json + rbegin/rend") {
                typename json::reverse_iterator it_begin = j.rbegin();
                typename json::reverse_iterator it_end = j.rend();

                auto it = it_begin;
                CHECK(it != it_end);

                if (it.base().decrement_support()) {
                    CHECK(*it == j["C"]);

                    it++;
                    CHECK(it != it_begin);
                    CHECK(it != it_end);
                    CHECK(*it == j["B"]);

                    ++it;
                    CHECK(it != it_begin);
                    CHECK(it != it_end);
                    CHECK(*it == j["A"]);

                    ++it;
                    CHECK(it != it_begin);
                    CHECK(it == it_end);
                }
            }

            SUBCASE("json + crbegin/crend") {
                typename json::const_reverse_iterator it_begin = j.crbegin();
                typename json::const_reverse_iterator it_end = j.crend();

                auto it = it_begin;
                CHECK(it != it_end);

                if (it.base().decrement_support()) {
                    CHECK(*it == j["C"]);

                    it++;
                    CHECK(it != it_begin);
                    CHECK(it != it_end);
                    CHECK(*it == j["B"]);

                    ++it;
                    CHECK(it != it_begin);
                    CHECK(it != it_end);
                    CHECK(*it == j["A"]);

                    ++it;
                    CHECK(it != it_begin);
                    CHECK(it == it_end);
                }
            }

            SUBCASE("const json + crbegin/crend") {
                typename json::const_reverse_iterator it_begin = j_const.crbegin();
                typename json::const_reverse_iterator it_end = j_const.crend();

                auto it = it_begin;
                CHECK(it != it_end);

                if (it.base().decrement_support()) {
                    CHECK(*it == j["C"]);

                    it++;
                    CHECK(it != it_begin);
                    CHECK(it != it_end);
                    CHECK(*it == j["B"]);

                    ++it;
                    CHECK(it != it_begin);
                    CHECK(it != it_end);
                    CHECK(*it == j["A"]);

                    ++it;
                    CHECK(it != it_begin);
                    CHECK(it == it_end);
                }
            }

            SUBCASE("key/value") {
                auto it = j.begin();
                auto cit = j_const.cbegin();
                CHECK(it.key() == "A");
                CHECK(it.value() == json(1));
                CHECK(cit.key() == "A");
                CHECK(cit.value() == json(1));
            }
        }

        SUBCASE("number (integer)") {
            json j {23};
            json const j_const(j);

            SUBCASE("json + begin/end") {
                typename json::iterator it = j.begin();
                CHECK(it != j.end());
                CHECK(*it == j);

                it++;
                CHECK(it != j.begin());
                CHECK(it == j.end());

                it--;
                CHECK(it == j.begin());
                CHECK(it != j.end());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.begin());
                CHECK(it == j.end());

                --it;
                CHECK(it == j.begin());
                CHECK(it != j.end());
                CHECK(*it == j);
            }

            SUBCASE("const json + begin/end") {
                typename json::const_iterator it = j_const.begin();
                CHECK(it != j_const.end());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.begin());
                CHECK(it == j_const.end());

                it--;
                CHECK(it == j_const.begin());
                CHECK(it != j_const.end());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.begin());
                CHECK(it == j_const.end());

                --it;
                CHECK(it == j_const.begin());
                CHECK(it != j_const.end());
                CHECK(*it == j_const);
            }

            SUBCASE("json + cbegin/cend") {
                typename json::const_iterator it = j.cbegin();
                CHECK(it != j.cend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.cbegin());
                CHECK(it == j.cend());

                it--;
                CHECK(it == j.cbegin());
                CHECK(it != j.cend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.cbegin());
                CHECK(it == j.cend());

                --it;
                CHECK(it == j.cbegin());
                CHECK(it != j.cend());
                CHECK(*it == j);
            }

            SUBCASE("const json + cbegin/cend") {
                typename json::const_iterator it = j_const.cbegin();
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.cbegin());
                CHECK(it == j_const.cend());

                it--;
                CHECK(it == j_const.cbegin());
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.cbegin());
                CHECK(it == j_const.cend());

                --it;
                CHECK(it == j_const.cbegin());
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);
            }

            SUBCASE("json + rbegin/rend") {
                typename json::reverse_iterator it = j.rbegin();
                CHECK(it != j.rend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.rbegin());
                CHECK(it == j.rend());

                it--;
                CHECK(it == j.rbegin());
                CHECK(it != j.rend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.rbegin());
                CHECK(it == j.rend());

                --it;
                CHECK(it == j.rbegin());
                CHECK(it != j.rend());
                CHECK(*it == j);
            }

            SUBCASE("json + crbegin/crend") {
                typename json::const_reverse_iterator it = j.crbegin();
                CHECK(it != j.crend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.crbegin());
                CHECK(it == j.crend());

                it--;
                CHECK(it == j.crbegin());
                CHECK(it != j.crend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.crbegin());
                CHECK(it == j.crend());

                --it;
                CHECK(it == j.crbegin());
                CHECK(it != j.crend());
                CHECK(*it == j);
            }

            SUBCASE("const json + crbegin/crend") {
                typename json::const_reverse_iterator it = j_const.crbegin();
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.crbegin());
                CHECK(it == j_const.crend());

                it--;
                CHECK(it == j_const.crbegin());
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.crbegin());
                CHECK(it == j_const.crend());

                --it;
                CHECK(it == j_const.crbegin());
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);
            }

            SUBCASE("key/value") {
                auto it = j.begin();
                auto cit = j_const.cbegin();
                CHECK_THROWS_WITH_AS(it.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(it.value() == json(42));
                CHECK_THROWS_WITH_AS(cit.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(cit.value() == json(42));

                auto rit = j.rend();
                auto crit = j.crend();

                CHECK_THROWS_WITH_AS(rit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(rit.base().value() == json(42));
                CHECK_THROWS_WITH_AS(crit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(crit.base().value() == json(42));
            }
        }

        SUBCASE("number (real)") {
            json j {23.42};
            json const j_const(j);

            SUBCASE("json + begin/end") {
                typename json::iterator it = j.begin();
                CHECK(it != j.end());
                CHECK(*it == j);

                it++;
                CHECK(it != j.begin());
                CHECK(it == j.end());

                it--;
                CHECK(it == j.begin());
                CHECK(it != j.end());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.begin());
                CHECK(it == j.end());

                --it;
                CHECK(it == j.begin());
                CHECK(it != j.end());
                CHECK(*it == j);
            }

            SUBCASE("const json + begin/end") {
                typename json::const_iterator it = j_const.begin();
                CHECK(it != j_const.end());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.begin());
                CHECK(it == j_const.end());

                it--;
                CHECK(it == j_const.begin());
                CHECK(it != j_const.end());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.begin());
                CHECK(it == j_const.end());

                --it;
                CHECK(it == j_const.begin());
                CHECK(it != j_const.end());
                CHECK(*it == j_const);
            }

            SUBCASE("json + cbegin/cend") {
                typename json::const_iterator it = j.cbegin();
                CHECK(it != j.cend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.cbegin());
                CHECK(it == j.cend());

                it--;
                CHECK(it == j.cbegin());
                CHECK(it != j.cend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.cbegin());
                CHECK(it == j.cend());

                --it;
                CHECK(it == j.cbegin());
                CHECK(it != j.cend());
                CHECK(*it == j);
            }

            SUBCASE("const json + cbegin/cend") {
                typename json::const_iterator it = j_const.cbegin();
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.cbegin());
                CHECK(it == j_const.cend());

                it--;
                CHECK(it == j_const.cbegin());
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.cbegin());
                CHECK(it == j_const.cend());

                --it;
                CHECK(it == j_const.cbegin());
                CHECK(it != j_const.cend());
                CHECK(*it == j_const);
            }

            SUBCASE("json + rbegin/rend") {
                typename json::reverse_iterator it = j.rbegin();
                CHECK(it != j.rend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.rbegin());
                CHECK(it == j.rend());

                it--;
                CHECK(it == j.rbegin());
                CHECK(it != j.rend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.rbegin());
                CHECK(it == j.rend());

                --it;
                CHECK(it == j.rbegin());
                CHECK(it != j.rend());
                CHECK(*it == j);
            }

            SUBCASE("json + crbegin/crend") {
                typename json::const_reverse_iterator it = j.crbegin();
                CHECK(it != j.crend());
                CHECK(*it == j);

                it++;
                CHECK(it != j.crbegin());
                CHECK(it == j.crend());

                it--;
                CHECK(it == j.crbegin());
                CHECK(it != j.crend());
                CHECK(*it == j);

                ++it;
                CHECK(it != j.crbegin());
                CHECK(it == j.crend());

                --it;
                CHECK(it == j.crbegin());
                CHECK(it != j.crend());
                CHECK(*it == j);
            }

            SUBCASE("const json + crbegin/crend") {
                typename json::const_reverse_iterator it = j_const.crbegin();
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);

                it++;
                CHECK(it != j_const.crbegin());
                CHECK(it == j_const.crend());

                it--;
                CHECK(it == j_const.crbegin());
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);

                ++it;
                CHECK(it != j_const.crbegin());
                CHECK(it == j_const.crend());

                --it;
                CHECK(it == j_const.crbegin());
                CHECK(it != j_const.crend());
                CHECK(*it == j_const);
            }

            SUBCASE("key/value") {
                auto it = j.begin();
                auto cit = j_const.cbegin();
                CHECK_THROWS_WITH_AS(it.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(it.value() == json(23.42));
                CHECK_THROWS_WITH_AS(cit.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(cit.value() == json(23.42));

                auto rit = j.rend();
                auto crit = j.crend();

                CHECK_THROWS_WITH_AS(rit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(rit.base().value() == json(23.42));
                CHECK_THROWS_WITH_AS(crit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(crit.base().value() == json(23.42));
            }
        }

        SUBCASE("null") {
            json j {nullptr};
            json const j_const(j);

            SUBCASE("json + begin/end") {
                typename json::iterator it = j.begin();
                CHECK(++it == j.end());
            }

            SUBCASE("const json + begin/end") {
                typename json::const_iterator it_begin = j_const.begin();
                typename json::const_iterator it_end = j_const.end();
                CHECK(++it_begin == it_end);
            }

            SUBCASE("json + cbegin/cend") {
                typename json::const_iterator it_begin = j.cbegin();
                typename json::const_iterator it_end = j.cend();
                CHECK(++it_begin == it_end);
            }

            SUBCASE("const json + cbegin/cend") {
                typename json::const_iterator it_begin = j_const.cbegin();
                typename json::const_iterator it_end = j_const.cend();
                CHECK(++it_begin == it_end);
            }

            SUBCASE("json + rbegin/rend") {
                typename json::reverse_iterator it = j.rbegin();
                CHECK(++it == j.rend());
            }

            SUBCASE("json + crbegin/crend") {
                typename json::const_reverse_iterator it = j.crbegin();
                CHECK(++it == j.crend());
            }

            SUBCASE("const json + crbegin/crend") {
                typename json::const_reverse_iterator it = j_const.crbegin();
                CHECK(++it == j_const.crend());
            }

            SUBCASE("key/value") {
                auto it = j.begin();
                auto cit = j_const.cbegin();
                CHECK_THROWS_WITH_AS(it.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(it.value() == json{nullptr});
                CHECK_THROWS_WITH_AS(cit.key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(cit.value() == json{nullptr});

                auto rit = j.rend();
                auto crit = j.crend();
                CHECK_THROWS_WITH_AS(rit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(rit.base().value() == json(nullptr));
                CHECK_THROWS_WITH_AS(crit.base().key(), make_error_code(jeyson::errc::incopatible_type).message().c_str(), jeyson::error);
                CHECK(crit.base().value() == json(nullptr));
            }
        }
    }

    SUBCASE("conversion from iterator to const iterator") {
        SUBCASE("boolean") {
            json j {true};
            typename json::const_iterator it = j.begin();
            CHECK(it == j.cbegin());
            it = j.begin();
            CHECK(it == j.cbegin());
        }

        SUBCASE("string") {
            json j {"hello world"};
            typename json::const_iterator it = j.begin();
            CHECK(it == j.cbegin());
            it = j.begin();
            CHECK(it == j.cbegin());
        }

        SUBCASE("array") {
            json j;
            j.push_back(1);
            j.push_back(2);
            j.push_back(3);
            typename json::const_iterator it = j.begin();

            CHECK(it == j.cbegin());
            it = j.begin();
            CHECK(it == j.cbegin());
        }

        SUBCASE("object") {
            json j;
            j.insert("A", 1);
            j.insert("B", 2);
            j.insert("C", 3);
            typename json::const_iterator it = j.begin();
            CHECK(it == j.cbegin());
            it = j.begin();
            CHECK(it == j.cbegin());
        }

        SUBCASE("number (integer)") {
            json j {23};
            typename json::const_iterator it = j.begin();
            CHECK(it == j.cbegin());
            it = j.begin();
            CHECK(it == j.cbegin());
        }

        SUBCASE("number (real)") {
            json j {23.42};
            typename json::const_iterator it = j.begin();
            CHECK(it == j.cbegin());
            it = j.begin();
            CHECK(it == j.cbegin());
        }

        SUBCASE("null") {
            json j {nullptr};
            typename json::const_iterator it = j.begin();
            CHECK(it == j.cbegin());
            it = j.begin();
            CHECK(it == j.cbegin());
        }
    }
}

TEST_CASE("Iterators") {
    run_iterator_tests<jeyson::backend::jansson>();
}
