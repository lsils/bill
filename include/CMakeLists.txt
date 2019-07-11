add_library(bill INTERFACE)
target_include_directories(bill INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(bill INTERFACE sat_solvers)
