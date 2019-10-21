# Distributed under the MIT License (See accompanying file /LICENSE)

add_library(sat_solvers INTERFACE)
target_include_directories(sat_solvers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sat_solvers ${CMAKE_CURRENT_SOURCE_DIR}/sat_solvers/sat_solvers)

if (NOT TARGET fmt)
  add_library(fmt INTERFACE)
  target_compile_definitions(fmt INTERFACE BILL_FMT_INTERNAL FMT_HEADER_ONLY)
  target_include_directories(fmt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/fmt)
endif()
