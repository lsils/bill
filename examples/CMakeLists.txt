# Distributed under the MIT License (See accompanying file /LICENSE)
# Author(s): Bruno Schmitt
add_compile_options(
  -Wall
  -pedantic)

add_custom_target(examples COMMENT "Build all examples.")

file(GLOB FILENAMES *.cpp)

foreach(filename ${FILENAMES})
  get_filename_component(basename ${filename} NAME_WE)
  add_executable(${basename} ${filename})
  target_link_libraries(${basename} PUBLIC bill)
  add_dependencies(examples ${basename})
endforeach()