#include <iostream>
#include <fstream>
#include <set>

#include <nlohmann/json.hpp>
#include <nlohmann/json-validator.hpp>

using namespace nlohmann;
using value_t = validation::value_t;

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
  validation::ignored_t ignored;
  validation::object()
    .with_number("id").in_range<int32_t>(0, 10).back()
    .with_string("name").back()
    .with_string("surname").optional().back()
    .with_object("auth")
      .with_string("nick").in_set<std::string>({"atom"}).back()
      .with_string("pass").back()
      .back()
    .with_boolean("enabled").back()
    .with_array("tokens").back()
    .exec(data, errors_map, ignored);

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
  if (!ignored.empty()) {
    std::cout << "ignored:\n";
    for (auto &&pointer : ignored) {
      std::cout << "  " << std::quoted(pointer.to_string()) << "\n";
    }
  }
  return EXIT_SUCCESS;
}