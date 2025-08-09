# Copyright (c) 2025 Furzoom.com, All rights reserved.
# Author: Niz, mn@furzoom.com

cutio_cxx_static_library(log
    "${cutio_cxx_strict}" # CXXFLAGS
    "yaml-cpp" # libs
    src/config.cc
    src/log_controller.cc
)

target_include_directories(log
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(log yaml-cpp)

cutio_cxx_executable_exclude_from_all(log_demo src "log yaml-cpp")