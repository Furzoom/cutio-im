# Copyright (c) 2025 Furzoom.com, All rights reserved.
# Author: Niz, mn@furzoom.com

cutio_cxx_static_library(net_filter
    "${cutio_cxx_strict}" # CXXFLAGS
    "" # libs
    src/net_filter_factory.cc
    src/packet.cc
    src/package_filter.cc
)

target_include_directories(net_filter
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)


