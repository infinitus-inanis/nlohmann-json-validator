cmake_minimum_required(VERSION 3.13...3.14)

add_library(test-main OBJECT src/unit.cpp)
target_compile_features(test-main
  PRIVATE
    cxx_std_11)
target_compile_definitions(test-main
  PUBLIC
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS)
target_include_directories(test-main
  PUBLIC
    thirdparty
    ${PROJECT_BINARY_DIR}/include)
target_link_libraries(test-main
  PUBLIC
    ${JSON_V_TARGET_NAME})

file(GLOB files src/unit-*.cpp)
foreach(file ${files})
  get_filename_component(file_basename ${file} NAME_WE)
  string(REGEX REPLACE "unit-([^$]+)" "test-\\1" test_name ${file_basename})

  set(test_target ${test_name})
  if (TARGET ${test_target})
    message(FATAL_ERROR "Target ${test_target} has already been added.")
  endif()

  add_executable(${test_target} ${file})
  target_link_libraries(${test_target}
    PRIVATE test-main)

  add_test(NAME ${test_target}
    COMMAND           ${test_target} ${DOCTEST_TEST_FILTER} --no-skip
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

endforeach()
