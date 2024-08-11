add_executable(example ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(example ${JSON_V_TARGET_NAME})
target_include_directories(example PRIVATE ${JSON_V_INCLUDE_BUILD_DIR})