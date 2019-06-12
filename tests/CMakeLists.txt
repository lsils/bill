# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : bill library tests
add_compile_options(
  -fno-rtti
  -Wall
  -pedantic)

include_directories(catch2)

#  All Tests
#  =============================================================================
add_custom_target(bill_tests COMMENT "Build all tests.")

set(bill_tests_files
  "${CMAKE_CURRENT_SOURCE_DIR}/run_tests.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/sat.cpp"
  )

add_executable(run_tests "${bill_tests_files}")
target_link_libraries(run_tests bill)
add_dependencies(bill_tests run_tests)
