#include "doctest/doctest_compatibility.h"

#include <nlohmann/json.hpp>
#include <nlohmann/json-validator.hpp>

TEST_CASE("nlohman json existence") {
  auto test = nlohmann::json{};
  REQUIRE_EQ(test.is_null(), true);
}