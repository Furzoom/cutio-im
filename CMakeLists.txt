# Copyright (c) 2025 Furzoom.com, All rights reserved.
# Author: Niz, mn@furzoom.com

cmake_minimum_required(VERSION 3.12)

project(cutio-im CXX)

include(cmake/internal_utils.cmake)

cutio_config_compiler_and_linker()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

option(ENABLE_TRACER "enable tracer?" OFF)
option(ENABLE_TESTS "enable build tests?" ON)

set(cutio_package_name cutio-im CACHE INTERNAL "")

set(INSTALL_GTEST OFF CACHE BOOL "Enable installation of googletest.")

if(ENABLE_TRACER)
  add_definitions(-DTRACER=1)
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    SYSTEM ${CMAKE_SOURCE_DIR}/build/local/include
    SYSTEM ${CMAKE_SOURCE_DIR}/common/packets/googletest/googletest/include
    SYSTEM ${CMAKE_SOURCE_DIR}/common/packets/googletest/googlemock/include
)

link_directories(
    ${CMAKE_SOURCE_DIR}/build/local/lib
)

add_subdirectory(server)
add_subdirectory(common)

if(ENABLE_TESTS)
  add_subdirectory(common/packets/googletest EXCLUDE_FROM_ALL)
  add_subdirectory(test)
endif()
