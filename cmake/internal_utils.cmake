# Copyright (c) 2025 Furzoom.com, All rights reserved.
# Author: Niz, mn@furzoom.com

macro(cutio_config_compiler_and_linker)
  set(cutio_cxx_base_flags "-g -Wall -Wextra -Werror -Wshadow -Wconversion -Wold-style-cast -Woverloaded-virtual -Wno-deprecated-copy -Wno-unused-parameter")
  set(cutio_cxx_exception_flags "-fexceptions")
  set(cutio_cxx_no_exception_flags "-fno-exceptions")
  set(cutio_cxx_no_rtti_flags "-fno-rtti")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(cutio_cxx_base_flags "${cutio_cxx_base_flags} -Wthread-safety -Wno-unknown-warning-option")
    set(cutio_cxx_strict_flags "-W -Wpointer-arith -Wreturn-type -Wcast-qual -Wwrite-strings -Wswitch -Wunused-parameter -Wcast-align -Wchar-subscripts -Winline -Wredundant-decls")
  elseif(CMAKE_COMPILER_IS_GNUCXX)
    if(NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0.0)
      set(cutio_cxx_base_flags "${cutio_cxx_base_falgs} -Wno-error=dangling-else")
    endif()
    set(cutio_cxx_strict_flags "-Wno-missing-field-initializers")
  endif()

  string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)
  if(build_type STREQUAL "DEBUG")
    set(cutio_cxx_base_flags "-O0 -D_DEBUG ${cutio_cxx_base_flags}")
  elseif(build_type STREQUAL "RELEASE")
    set(cutio_cxx_base_flags "-O2 -DNDEBUG ${cutio_cxx_base_flags}")
  else()
    set(cutio_cxx_base_flags "-O2 -DNDEBUG ${cutio_cxx_base_flags}")
  endif()

  set(cutio_cxx_exception "${cutio_cxx_base_flags} ${cutio_cxx_exception_flags}")
  set(cutio_cxx_no_exception "${CMAKE_CXX_FLAGS} ${cutio_cxx_base_flags} ${cutio_cxx_no_exception_flags}")
  set(cutio_cxx_default "${cutio_cxx_exception}")
  set(cutio_cxx_no_rtti "${cutio_cxx_default} ${cutio_cxx_no_rtti_flags}")
  set(cutio_cxx_strict "${cutio_cxx_default} ${cutio_cxx_strict_flags}")
endmacro()

function(cutio_cxx_library_with_type name type cxx_flags libs)
  # type can by either STATIC or SHARED to denote a static or shared library.
  # ARGN refers to additional arguments after 'cxx_flags'.
  add_library(${name} ${type} ${ARGN})
  add_library(${cutio}::${name} ALIAS ${name})
  set_target_properties(${name}
      PROPERTIES
      COMPILE_FLAGS "${cxx_flags}")
  if (type STREQUAL "STATIC")
    set_target_properties(${name}
        PROPERTIES
        POSITION_INDEPENDENT_CODE ON)
  endif()

  if(NOT libs STREQUAL "")
    string(REPLACE " " ";" link_libs ${libs})
    foreach(lib ${link_libs})
      target_link_libraries(${name} ${lib})
    endforeach()
  endif()

  target_compile_features(${name} PUBLIC cxx_std_20)
endfunction()

function(cutio_cxx_executable_with_flags name cxx_flags libs)
  add_executable(${name} ${ARGN})
  set_target_properties(${name}
      PROPERTIES
      COMPILE_FLAGS "${cxx_flags}")

  if(NOT libs STREQUAL "")
    string(REPLACE " " ";" link_libs ${libs})
    foreach(lib ${link_libs})
      target_link_libraries(${name} ${lib})
    endforeach()
  endif()

  target_compile_features(${name} PUBLIC cxx_std_20)
endfunction()


################################################################################
#
# Helper functions for creating build targets.

function(cutio_cxx_shared_library name cxx_flags libs)
  cutio_cxx_library_with_type(${name} SHARED "${cxx_flags}" "${libs}" ${ARGN})
endfunction()

function(cutio_cxx_static_library name cxx_flags libs)
  cutio_cxx_library_with_type(${name} STATIC "${cxx_flags}" "${libs}" ${ARGN})
endfunction()

function(cutio_cxx_executable name dir libs)
  cutio_cxx_executable_with_flags(
      ${name} "${cutio_cxx_strict}" "${libs}" "${dir}/${name}.cc" ${ARGN})
endfunction()

function(cutio_cxx_executable_exclude_from_all name dir libs)
  cutio_cxx_executable_with_flags(
      ${name} "${cutio_cxx_strict}" "${libs}" "${dir}/${name}.cc" ${ARGN})
  set_target_properties(${name} PROPERTIES EXCLUDE_FROM_ALL TRUE)
endfunction()
