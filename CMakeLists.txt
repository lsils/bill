# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : bill library
cmake_minimum_required(VERSION 3.8)
project(bill LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(UNIX)
  include(CheckCXXCompilerFlag)
  # show quite some warnings (but remove some intentionally)
  add_compile_options(-Wall -Wextra)
  foreach (WARNING unknown-pragmas gnu-anonymous-struct nested-anon-types)
    check_cxx_compiler_flag("-Wno-${WARNING}" HAS_WNO_${WARNING})
    if (HAS_WNO_${WARNING})
      add_compile_options(-Wno-${WARNING})
    endif()
  endforeach()
endif()

# Options
# =============================================================================
option(BILL_DOCS "Build documentation" OFF)
option(BILL_EXAMPLES "Build examples" ON)
option(BILL_EXPERIMENTS "Build experiments" OFF)
option(BILL_TESTS "Build tests" OFF)

# Libs
# =============================================================================
add_subdirectory(libs)

# Library
# =============================================================================
add_library(bill INTERFACE)
target_include_directories(bill INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(bill INTERFACE sat_solvers)

# Documentation
# =============================================================================
if(BILL_DOCS)
  add_subdirectory(docs)
endif()

# Examples
# =============================================================================
if(BILL_EXAMPLES)
  add_subdirectory(examples)
endif()

# Experiments
# =============================================================================
if(BILL_EXPERIMENTS)
  add_subdirectory(experiments)
endif()

# Tests
# =============================================================================
if(BILL_TESTS)
  add_subdirectory(tests)
endif()
