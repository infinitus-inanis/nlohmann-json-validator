#include <iostream>
#include <fstream>
#include <set>

#include <nlohmann/json.hpp>
#include <nlohmann/json-validator.hpp>

using namespace nlohmann;
using value_t = validation::value_t;

struct set_match_rule : public validation::type_rule {
  set_match_rule(std::set<std::string> values)
    : type_rule(validation::value_t::string)
    , _values{std::move(values)}
  {}

  bool operator() (const nlohmann::json &json, validation::errors_collector_t &errors) const override {
    if (!type_rule::operator()(json, errors))
      return false;

    if (!_values.contains(json.get_ref<const std::string &>())) {
      errors.emplace("value set match failure");
      return false;
    }
    return true;
  }

private:
  std::set<std::string> _values;
};

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "path to validation data argument is missing\n";
    return EXIT_FAILURE;
  }
  auto path = argv[1];

  auto file = std::ifstream{path};
  if (!file) {
    std::cerr << "failed to open: " << path << "\n";
    return EXIT_FAILURE;
  }
  auto data = json::parse(file, nullptr, true, true);
  std::cout << "data: " << data.dump(2) << "\n\n";

  validation::errors_map_t errors_map;
  validation::object()
    .with_value("id").with_type(value_t::number_unsigned).next()
    .with_string("name").next()
    .with_string("surname").optional().next()
    .with_object("auth")
      .with_string("nick")
        .with_rule<set_match_rule>(std::set<std::string>{"atom"})
        .next()
      .with_string("pass").next()
      .next()
    .with_boolean("enabled").next()
    .exec(data, errors_map);

  if (!errors_map.empty()) {
    std::cout << "validation failure:\n";
    for (auto &&[pointer, errors] : errors_map) {
      std::cout << "  " << std::quoted(pointer.to_string()) << "\n";
      for (auto &&error : errors) {
        std::cout << "    - " << error.message() << "\n";
      }
    }
  } else {
    std::cout << "validation success.\n";
  }
  return EXIT_SUCCESS;
}