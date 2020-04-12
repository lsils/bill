# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : bill library tests
add_compile_options(
  -fno-rtti
  -Wall
  -pedantic)

include_directories(catch2)

#  All Tests
#  =============================================================================
add_custom_target(bill_test COMMENT "Build all tests.")

file(GLOB_RECURSE FILENAMES *.cpp)

add_executable(run_tests "${FILENAMES}")

if (ENABLE_COVERAGE)
  target_link_libraries(run_tests bill gcov)
else()
  target_link_libraries(run_tests bill)
endif()

add_dependencies(bill_test run_tests)

