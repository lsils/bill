cmake_minimum_required(VERSION 2.8.12)

# In-source build prevention.
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(STATUS "Project requires an out-of-source build.")
  message(STATUS "Please remove these files from ${CMAKE_BINARY_DIR} first:")
  message(STATUS "CMakeCache.txt")
  message(STATUS "CMakeFiles")
  message(STATUS "Once these files are removed, create a separate directory")
  message(STATUS "and run CMake from there.")
  message(FATAL_ERROR "in-source build detected")
endif()

project(CUDD LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VERSION "3.0.0")

####################### Begin Options ###################

option(CUDD_BUILD_TESTS "Build the tests" ON)
option(CUDD_ENABLE_DDDMP "Include libdddmp in libcudd" ON)
option(CUDD_ENABLE_OBJ "Include C++ libobj in libcudd" ON)
option(CUDD_ENABLE_SHARED "Build shared libraries" OFF)
option(CUDD_WITH_SYSTEM_QSORT "Use system qsort instead of portable one" OFF)

set(BUILD_SHARED_LIBS "${CUDD_ENABLE_SHARED}")

####################### End Options ###################

####################### Begin compiler configurations ###################

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

if(UNIX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

include(CheckCXXCompilerFlag)

if(CUDD_ENABLE_OBJ)
  # include(CheckCXXCompilerFlag)
  # CHECK_CXX_COMPILER_FLAG("-std=c++17" COMPILER_SUPPORTS_CXX17)
  # if(NOT COMPILER_SUPPORTS_CXX17)
  #   message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++17 support.")
  # endif()
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  include(CheckCXXSourceCompiles)
  # CHECK_CXX_SOURCE_COMPILES(source variable)
  set(HAVE_MODERN_CXX TRUE)
  set(HAVE_WORKING_THREAD TRUE)
endif()

if(CUDD_WITH_SYSTEM_QSORT)
  set(USE_SYSTEM_QSORT TRUE)
endif()

# set(__USE_MINGW_ANSI_STDIO ON)  # TODO: Test.

find_package(Threads)
if(CMAKE_USE_PTHREADS_INIT)
  add_definitions(-DHAVE_PTHREADS=1)
  set(HAVE_PTHREADS TRUE)
  add_definitions("-pthread")  # TODO: Revise per target.
  # target_link_libraries(${name} ${CMAKE_THREAD_LIBS_INIT})
endif()

include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("float.h" HAVE_FLOAT_H)
CHECK_INCLUDE_FILE_CXX("inttypes.h" HAVE_INTTYPES_H)
CHECK_INCLUDE_FILE_CXX("limits.h" HAVE_LIMITS_H)
CHECK_INCLUDE_FILE_CXX("stddef.h" HAVE_STDDEF_H)
CHECK_INCLUDE_FILE_CXX("stdlib.h" HAVE_STDLIB_H)
CHECK_INCLUDE_FILE_CXX("string.h" HAVE_STRING_H)
CHECK_INCLUDE_FILE_CXX("assert.h" HAVE_ASSERT_H)
CHECK_INCLUDE_FILE_CXX("math.h" HAVE_MATH_H)

if(NOT (HAVE_FLOAT_H AND HAVE_INTTYPES_H AND HAVE_LIMITS_H AND HAVE_STDDEF_H
        AND HAVE_STDLIB_H AND HAVE_STRING_H AND HAVE_ASSERT_H AND HAVE_MATH_H))
  message(FATAL_ERROR "One or more mandatory headers missing.")
endif()

CHECK_INCLUDE_FILE_CXX("unistd.h" HAVE_UNISTD_H)
CHECK_INCLUDE_FILE_CXX("sys/time.h" HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILE_CXX("sys/times.h" HAVE_SYS_TIMES_H)
CHECK_INCLUDE_FILE_CXX("sys/resource.h" HAVE_SYS_RESOURCE_H)
CHECK_INCLUDE_FILE_CXX("sys/wait.h" HAVE_SYS_WAIT_H)
CHECK_INCLUDE_FILE_CXX("sys/stat.h" HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILE_CXX("dlfcn.h" HAVE_DLFCN_H)  # libtool
CHECK_INCLUDE_FILE_CXX("memory.h" HAVE_MEMORY_H)
CHECK_INCLUDE_FILE_CXX("strings.h" HAVE_STRINGS_H)

set(STDC_HEADERS TRUE)  # TODO: Test

CHECK_INCLUDE_FILE_CXX("stdbool.h" HAVE__BOOL) # TODO: same as AC_CHECK_HEADER_STDBOOL?

include(CheckTypeSize)
CHECK_TYPE_SIZE(size_t SIZE_T)
CHECK_TYPE_SIZE(uint16_t UINT16_T)
CHECK_TYPE_SIZE(uint32_t UINT32_T)
CHECK_TYPE_SIZE(ptrdiff_t PTRDIFF_T)
CHECK_TYPE_SIZE(int SIZEOF_INT)
CHECK_TYPE_SIZE(long SIZEOF_LONG)
CHECK_TYPE_SIZE("void*" SIZEOF_VOID_P)
CHECK_TYPE_SIZE("long double" SIZEOF_LONG_DOUBLE)

if(NOT MSVC)
  include(CheckFunctionExists)
  
  set(CMAKE_REQUIRED_INCLUDES "math.h;cmath;string.h;cstring")
  set(CMAKE_REQUIRED_LIBRARIES m)
  
    CHECK_FUNCTION_EXISTS(pow HAVE_POW LANGUAGE CXX)
    CHECK_FUNCTION_EXISTS(sqrt HAVE_SQRT LANGUAGE CXX)
    CHECK_FUNCTION_EXISTS(strchr HAVE_STRCHR LANGUAGE CXX)
    CHECK_FUNCTION_EXISTS(strstr HAVE_STRSTR LANGUAGE CXX)
    
    if(NOT (HAVE_POW AND HAVE_SQRT AND HAVE_STRCHR AND HAVE_STRSTR))
      
      message("${HAVE_POW}, ${HAVE_SQRT}, ${HAVE_STRCHR}, ${HAVE_STRSTR}")
      message(FATAL_ERROR "One or more mandatory functions missing.")
    endif()

    CHECK_FUNCTION_EXISTS(powl HAVE_POWL LANGUAGE CXX)
    CHECK_FUNCTION_EXISTS(gethostname HAVE_GETHOSTNAME LANGUAGE CXX)
    CHECK_FUNCTION_EXISTS(getrlimit HAVE_GETRLIMIT LANGUAGE CXX)
    CHECK_FUNCTION_EXISTS(getrusage HAVE_GETRUSAGE LANGUAGE CXX)
    CHECK_FUNCTION_EXISTS(sysconf HAVE_SYSCONF LANGUAGE CXX)
else()
    set(HAVE_POWL TRUE)
endif()


include(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES(
  "#include <math.h>
  int main() { double x = INFINITY; }
  " HAVE_IEEE_754)

######################## End compiler configurations ####################

##################### Begin cmake configuration ###################

# Set project directories.
set(PROJECT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(CUDD_SOURCE_DIR "${PROJECT_SOURCE_DIR}/cudd")
set(CUDD_CPP_DIR "${PROJECT_SOURCE_DIR}/cplusplus")
set(CUDD_UTIL_DIR "${PROJECT_SOURCE_DIR}/util")
set(CUDD_EPD_DIR "${PROJECT_SOURCE_DIR}/epd")
set(CUDD_MTR_DIR "${PROJECT_SOURCE_DIR}/mtr")
set(CUDD_ST_DIR "${PROJECT_SOURCE_DIR}/st")

######################## End cmake configuration ###################

########################## Begin includes ###############################

configure_file(config-cmake.h.in "config.h" @ONLY)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory("${CUDD_EPD_DIR}")
add_subdirectory("${CUDD_MTR_DIR}")
add_subdirectory("${CUDD_ST_DIR}")
add_subdirectory("${CUDD_UTIL_DIR}")
add_subdirectory("${CUDD_CPP_DIR}")
add_subdirectory("${CUDD_SOURCE_DIR}")

####################### End includes ####################################
