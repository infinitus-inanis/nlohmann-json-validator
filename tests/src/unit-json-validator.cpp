#include "doctest/doctest_compatibility.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include <nlohmann/json-validator.hpp>

using namespace nlohmann;

TEST_SUITE_BEGIN("nlohman json validator");

TEST_CASE("base functionality") {
  SECTION("subcase result (success)") {
    auto test = json::object();
    auto result = validation::object().exec(test);
    REQUIRE(result);
  }
  SECTION("subcase result (failure)") {
    auto test = json::object();
    auto result = validation::array().exec(test);
    REQUIRE_FALSE(result);
  }
  SECTION("subcase result with errors_map (success)") {
    auto test = json::array();
    validation::errors_map_t errors_map;
    auto result = validation::array().exec(test, errors_map);
    REQUIRE(result);
    REQUIRE(errors_map.empty());
  }
  SECTION("subcase result with errors_map (failure)") {
    auto test = json::array();
    validation::errors_map_t errors_map;
    auto result = validation::object().exec(test, errors_map);
    REQUIRE_FALSE(result);
    REQUIRE_FALSE(errors_map.empty());
  }
}

TEST_CASE("error pointer") {
  const auto test = json{{ "test", {} }};
  const auto miss_type = validation::value_t::object;

  validation::errors_map_t errors_map;
  validation::object()
    .with_value("test")
      .with_type(miss_type)
      .next()
    .exec(test, errors_map);
  REQUIRE_EQ(errors_map.size(), 1);

  auto &&[pointer, errors] = *errors_map.begin();
  REQUIRE_EQ(errors.size(), 1);
  REQUIRE_NOTHROW(test.at(pointer));
}

TEST_CASE("object value (single)") {
  const auto test = json{{ "test", {} }};

  SECTION("subcase required (success)") {
    validation::errors_map_t errors_map;
    validation::object()
      .with_value("test").next()
      .exec(test, errors_map);
    REQUIRE_EQ(errors_map.size(), 0);
  }

  SECTION("subcase required (failure)") {
    validation::errors_map_t errors_map;
    validation::object()
      .with_value("miss").next()
      .exec(test, errors_map);
    REQUIRE_EQ(errors_map.size(), 1);

    auto &&[pointer, errors] = *errors_map.begin();
    REQUIRE_EQ(errors.size(), 1);
    REQUIRE_NOTHROW(test.at(pointer));
  }

  SECTION("subcase optional (success)") {
    validation::errors_map_t errors_map;
    validation::object()
      .with_value("miss")
        .optional()
        .next()
      .exec(test, errors_map);
    REQUIRE_EQ(errors_map.size(), 0);
  }
}

TEST_CASE("multiple object values (adjacent)") {
  const auto test = json{{ "test0", {} }, { "test1", {} }, { "test2", {} }};

  validation::errors_map_t errors_map;
  validation::object()
    .with_value("test0").next()
    .with_value("miss1").next()
    .with_value("miss2").optional().next()
    .exec(test, errors_map);
  REQUIRE_EQ(errors_map.size(), 1);
}

TEST_CASE("multiple object values (nested)") {
  auto test = json{{ "test0", {{ "test1", {{ "test2", nullptr }} }} }};

  SECTION("subcase (success)") {
    validation::errors_map_t errors_map;
    validation::object()
      .with_value("test0")
        .with_value("test1")
          .with_value("test2")
            .next().next().next()
      .exec(test, errors_map);
    REQUIRE_EQ(errors_map.size(), 0);
  }
  SECTION("subcase (failure)") {
    validation::errors_map_t errors_map;
    validation::object()
      .with_value("test0")
        .with_value("miss1")
          /* not reachable during validation */
          .with_value("test2")
            .next().next().next()
      .exec(test, errors_map);
    REQUIRE_EQ(errors_map.size(), 1);

    auto &&[pointer, errors] = *errors_map.begin();
    REQUIRE_EQ(errors.size(), 1);
    REQUIRE_NOTHROW(test.at(pointer));
  }
}

TEST_SUITE_END;