# Copyright (c) 2025 Furzoom.com, All rights reserved.
# Author: Niz, mn@furzoom.com

cutio_cxx_static_library(net_event
    "${cutio_cxx_strict}" # CXXFLAGS
    "" # libs
    src/io_service.cc
    src/io_thread.cc
    src/io_utils.cc
    src/mutex.cc
)

target_include_directories(net_event
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)