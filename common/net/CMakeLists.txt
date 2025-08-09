# Copyright (c) 2025 Furzoom.com, All rights reserved.
# Author: Niz, mn@furzoom.com

cutio_cxx_static_library(net_event
    "${cutio_cxx_strict}" # CXXFLAGS
    "" # libs
    src/base_io_thread.cc
    src/io_buffer.cc
    src/io_service.cc
    src/io_thread.cc
    src/io_utils.cc
    src/network_io.cc
    src/read_io_thread.cc
    src/session.cc
    src/udp_read_io_thread.cc
    src/udp_write_io_thread.cc
    src/write_io_thread.cc
)

target_include_directories(net_event
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cutio_cxx_executable_exclude_from_all(udp_server
    demo
    "net_event log event"
    demo/udp_server_data_trans.cc
    demo/udp_server_io_event.cc
)

cutio_cxx_executable_exclude_from_all(udp_client
    demo
    "net_event log event"
    demo/udp_client.cc
    demo/udp_client_io_event.cc
)
