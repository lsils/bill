# Distributed under the MIT License (See accompanying file /LICENSE)

add_library(sat_solvers INTERFACE)
target_include_directories(sat_solvers INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/sat_solvers ${CMAKE_CURRENT_SOURCE_DIR}/sat_solvers/sat_solvers)
