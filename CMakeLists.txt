cmake_minimum_required(VERSION 3.1...3.14)

project(nlohmann_json_validator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# dependency
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(nlohmann/json)

# options
option(JSON_V_BuildExample "Build example" OFF)
# option(JSON_V_BuildTests "Build tests" OFF)

# configs
include(GNUInstallDirs)
set(JSON_V_TARGET_NAME ${PROJECT_NAME})
set(JSON_V_INCLUDE_BUILD_DIR ${PROJECT_SOURCE_DIR}/include)

# library
add_library(${JSON_V_TARGET_NAME} INTERFACE)
add_library(nlohmann_json::${JSON_V_TARGET_NAME} ALIAS ${JSON_V_TARGET_NAME})

target_include_directories(${JSON_V_TARGET_NAME}
  INTERFACE
    $<BUILD_INTERFACE:${JSON_V_INCLUDE_BUILD_DIR}>)

target_link_libraries(${JSON_V_TARGET_NAME}
  INTERFACE
    nlohmann_json::nlohmann_json)

if (JSON_V_BuildExample)
  add_subdirectory(example)
endif()

# tests
# if (JSON_V_BuildTests)
#   include(CTest)
#   enable_testing()
#   add_subdirectory(tests)
# endif()