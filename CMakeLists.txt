# Distributed under the MIT License (See accompanying file /LICENSE)
# CMake build : bill library
cmake_minimum_required(VERSION 3.8)
project(bill LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
# =============================================================================
option(BILL_DOCS "Build documentation" OFF)
option(BILL_EXAMPLES "Build examples" ON)
option(BILL_EXPERIMENTS "Build experiments" OFF)
option(BILL_TEST "Build tests" OFF)
option(BILL_Z3 "Enable Z3 interface" OFF)
option(ENABLE_COVERAGE "Enable coverage reporting for gcc/clang" OFF)

if(UNIX)
  # show quite some warnings (but remove some intentionally)
  include(CheckCXXCompilerFlag)
  add_compile_options(-Wall -Wextra)
  foreach (WARNING unknown-pragmas gnu-anonymous-struct nested-anon-types)
    check_cxx_compiler_flag("-Wno-${WARNING}" HAS_WNO_${WARNING})
    if (HAS_WNO_${WARNING})
      add_compile_options(-Wno-${WARNING})
    endif()
  endforeach()
  if (ENABLE_COVERAGE)
    add_compile_options(-O0 -g --coverage -fprofile-arcs -ftest-coverage)
  endif()
endif()

# Libs
# =============================================================================
add_subdirectory(lib)

# Library
# =============================================================================
add_library(bill INTERFACE)
target_include_directories(bill INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(bill INTERFACE fmt cudd cudd_includes)
if(WIN32)
  target_compile_definitions(bill INTERFACE NOMINMAX)
endif()

# Z3 (optional)
# =============================================================================
if(BILL_Z3)
  target_compile_definitions(bill INTERFACE BILL_HAS_Z3)
  set(BILL_Z3_INCLUDE_PATH "" CACHE PATH "Path to Z3 includes, e.g., z3++.h")
  set(BILL_Z3_LIBRARY_PATH "" CACHE PATH "Path to Z3 library, e.g., libz3.a")
  if(NOT "${BILL_Z3_INCLUDE_PATH}" STREQUAL "")
    target_include_directories(bill INTERFACE ${BILL_Z3_INCLUDE_PATH})
  endif()
  if(NOT "${BILL_Z3_LIBRARY_PATH}" STREQUAL "")
    target_link_directories(bill INTERFACE ${BILL_Z3_LIBRARY_PATH})
  endif()
  if (WIN32)
    target_link_libraries(bill INTERFACE libz3)
  else()
    target_link_libraries(bill INTERFACE z3)
  endif()
endif()

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
if(BILL_TEST)
  add_subdirectory(test)
endif()
