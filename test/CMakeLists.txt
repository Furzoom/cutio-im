# Copyright (c) 2025 Furzoom.com, All rights reserved.
# Author: Niz, mn@furzoom.com

set(COMMON_SRC
    common/log/config_unittest.cc
    common/log/log_controller_unittest.cc
)

set(UTILS_SRC
  test_utils/global_env.cc
)

cutio_cxx_executable(cutio-im-test "." "gtest log"
    ${COMMON_SRC}
    ${UTILS_SRC}
)

add_custom_command(TARGET cutio-im-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/testdata ${PROJECT_BINARY_DIR}/testdata
    COMMENT "copy ${CMAKE_CURRENT_SOURCE_DIR}/testdata to ${PROJECT_BINARY_DIR}/testdata"
    VERBATIM)
