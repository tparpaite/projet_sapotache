cmake_minimum_required(VERSION 2.8)
project(SAPOTACHE)
find_package(Doxygen)
SET(CMAKE_C_COMPILER gcc)

# CMake mode
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "DEBUG")
else(NOT CMAKE_BUILD_TYPE)
  string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
endif(NOT CMAKE_BUILD_TYPE)
message(STATUS "Mode : ${CMAKE_BUILD_TYPE}")


# Makefile configuration
add_definitions(-std=c99 -fPIC)#-Wall -Wextra -Werror) #-pedantic -pedantic-errors)

if(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")
  add_definitions(-g -O0)
endif(${CMAKE_BUILD_TYPE} MATCHES "DEBUG")


# Useful paths
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(TESTS_PATH ${EXECUTABLE_OUTPUT_PATH}/tests)
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/include/parsing
                    ${PROJECT_SOURCE_DIR}/include/ADT
                    ${PROJECT_SOURCE_DIR}/include/game)


# Doxygen
if(${DOXYGEN_FOUND})
  configure_file(${PROJECT_SOURCE_DIR}/Doxyfile ${PROJECT_BINARY_DIR}/doc/Doxyfile.cmake @ONLY)
  add_custom_target(doc
                    COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/doc/Doxyfile.cmake
                    COMMENT "Generating documentation using Doxygen ${DOXYGEN_VERSION}...")
  add_custom_target(showdoc
                    COMMAND firefox ${PROJECT_BINARY_DIR}/doc/html/index.html
                    COMMENT "Showing doc with firefox...")
endif(${DOXYGEN_FOUND})


# Tests
enable_testing()


add_subdirectory(src)
