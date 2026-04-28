#
# Copyright (C) 2026  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#

INCLUDE(ProcessorCount) # require CMake 3.15+
PROCESSORCOUNT(_cpu_count)

# Crashpad - https://github.com/getsentry/crashpad Using commit hash since the repo doesn't have versioned releases. mini_chromium is a git submodule, so we
# clone via GIT_REPOSITORY — no download hash needed.
SET(RV_DEPS_CRASHPAD_GIT_TAG
    "38617eb5a0799acade5dc4312f206e0e43642566"
)

RV_CREATE_STANDARD_DEPS_VARIABLES("RV_DEPS_CRASHPAD" "${RV_DEPS_CRASHPAD_GIT_TAG}" "" "")

# getsentry/crashpad is a fork of Google's Crashpad with native CMake support. mini_chromium is a git submodule (third_party/mini_chromium), so we must use
# GIT_REPOSITORY + GIT_SUBMODULES rather than a zip archive download. Note: Using commit hash since the repo doesn't have versioned releases.

# Include directories: - crashpad headers are at the source root (e.g. <client/crashpad_client.h>) - mini_chromium submodule checks out into
# third_party/mini_chromium/; within that repo, headers live under a nested mini_chromium/ subdirectory
SET(_crashpad_include_dir
    ${_source_dir}
)
SET(_mini_chromium_include_dir
    ${_source_dir}/third_party/mini_chromium/mini_chromium
)

# Library names
SET(_crashpad_client_lib_name
    ${CMAKE_STATIC_LIBRARY_PREFIX}crashpad_client${CMAKE_STATIC_LIBRARY_SUFFIX}
)
SET(_crashpad_util_lib_name
    ${CMAKE_STATIC_LIBRARY_PREFIX}crashpad_util${CMAKE_STATIC_LIBRARY_SUFFIX}
)
# getsentry/crashpad names the mini_chromium library with an underscore
SET(_minichromium_lib_name
    ${CMAKE_STATIC_LIBRARY_PREFIX}mini_chromium${CMAKE_STATIC_LIBRARY_SUFFIX}
)
# getsentry/crashpad splits mpack into its own library (not present in older forks)
SET(_crashpad_mpack_lib_name
    ${CMAKE_STATIC_LIBRARY_PREFIX}crashpad_mpack${CMAKE_STATIC_LIBRARY_SUFFIX}
)

# Library paths in install directory
SET(_crashpad_client_lib
    ${_lib_dir}/${_crashpad_client_lib_name}
)
SET(_crashpad_util_lib
    ${_lib_dir}/${_crashpad_util_lib_name}
)
SET(_minichromium_lib
    ${_lib_dir}/${_minichromium_lib_name}
)
SET(_crashpad_mpack_lib
    ${_lib_dir}/${_crashpad_mpack_lib_name}
)

# Handler executable
IF(RV_TARGET_WINDOWS)
  SET(_crashpad_handler_name
      crashpad_handler.exe
  )
ELSE()
  SET(_crashpad_handler_name
      crashpad_handler
  )
ENDIF()

SET(_crashpad_handler
    ${_bin_dir}/${_crashpad_handler_name}
)

# Configure options (CMAKE_INSTALL_PREFIX, CMAKE_BUILD_TYPE, CMAKE_OSX_ARCHITECTURES, -S and -B are already appended by RV_CREATE_STANDARD_DEPS_VARIABLES)
LIST(APPEND _configure_options "-DCMAKE_POSITION_INDEPENDENT_CODE=ON")
LIST(APPEND _configure_options "-DCRASHPAD_BUILD_TESTS=OFF")

# Disable deprecation warnings on macOS — mini_chromium uses deprecated Security APIs
IF(RV_TARGET_DARWIN)
  LIST(APPEND _configure_options "-DCMAKE_CXX_FLAGS=-Wno-error=deprecated-declarations -Wno-deprecated-declarations")
  LIST(APPEND _configure_options "-DCMAKE_C_FLAGS=-Wno-error=deprecated-declarations -Wno-deprecated-declarations")
ENDIF()

EXTERNALPROJECT_ADD(
  ${_target}
  GIT_REPOSITORY "https://github.com/getsentry/crashpad.git"
  GIT_TAG ${_version}
  GIT_SUBMODULES "third_party/mini_chromium"
  GIT_SHALLOW FALSE
  GIT_PROGRESS TRUE
  UPDATE_DISCONNECTED TRUE
  SOURCE_DIR ${_source_dir}
  BINARY_DIR ${_build_dir}
  INSTALL_DIR ${_install_dir}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} ${_configure_options}
  BUILD_COMMAND ${_cmake_build_command}
  INSTALL_COMMAND ${_cmake_install_command}
  BUILD_ALWAYS FALSE
  BUILD_BYPRODUCTS ${_crashpad_client_lib} ${_crashpad_util_lib} ${_minichromium_lib} ${_crashpad_mpack_lib} ${_crashpad_handler}
  USES_TERMINAL_BUILD TRUE
)

# Create imported target for crashpad::client
ADD_LIBRARY(crashpad::client STATIC IMPORTED GLOBAL)
ADD_DEPENDENCIES(crashpad::client ${_target})
SET_PROPERTY(
  TARGET crashpad::client
  PROPERTY IMPORTED_LOCATION ${_crashpad_client_lib}
)
SET_PROPERTY(
  TARGET crashpad::client
  PROPERTY IMPORTED_SONAME ${_crashpad_client_lib_name}
)

FILE(MAKE_DIRECTORY ${_crashpad_include_dir})
FILE(MAKE_DIRECTORY ${_mini_chromium_include_dir})
TARGET_INCLUDE_DIRECTORIES(
  crashpad::client
  INTERFACE ${_crashpad_include_dir} ${_mini_chromium_include_dir}
)

# crashpad_client depends on crashpad_mpack (split out by getsentry/crashpad); declare it here so consumers of crashpad::client pull it in automatically.
ADD_LIBRARY(crashpad::mpack STATIC IMPORTED GLOBAL)
ADD_DEPENDENCIES(crashpad::mpack ${_target})
SET_PROPERTY(
  TARGET crashpad::mpack
  PROPERTY IMPORTED_LOCATION ${_crashpad_mpack_lib}
)
SET_PROPERTY(
  TARGET crashpad::client
  PROPERTY INTERFACE_LINK_LIBRARIES crashpad::mpack
)

# Create imported target for crashpad::util
ADD_LIBRARY(crashpad::util STATIC IMPORTED GLOBAL)
ADD_DEPENDENCIES(crashpad::util ${_target})
SET_PROPERTY(
  TARGET crashpad::util
  PROPERTY IMPORTED_LOCATION ${_crashpad_util_lib}
)
SET_PROPERTY(
  TARGET crashpad::util
  PROPERTY IMPORTED_SONAME ${_crashpad_util_lib_name}
)

TARGET_INCLUDE_DIRECTORIES(
  crashpad::util
  INTERFACE ${_crashpad_include_dir} ${_mini_chromium_include_dir}
)

# Create imported target for crashpad::minichromium (base library)
ADD_LIBRARY(crashpad::minichromium STATIC IMPORTED GLOBAL)
ADD_DEPENDENCIES(crashpad::minichromium ${_target})
SET_PROPERTY(
  TARGET crashpad::minichromium
  PROPERTY IMPORTED_LOCATION ${_minichromium_lib}
)
SET_PROPERTY(
  TARGET crashpad::minichromium
  PROPERTY IMPORTED_SONAME ${_minichromium_lib_name}
)

TARGET_INCLUDE_DIRECTORIES(
  crashpad::minichromium
  INTERFACE ${_mini_chromium_include_dir}
)

# Add to deps list
LIST(APPEND RV_DEPS_LIST crashpad::client crashpad::util crashpad::minichromium)

# Stage libraries and crashpad_handler executable
IF(RV_TARGET_WINDOWS)
  ADD_CUSTOM_COMMAND(
    TARGET ${_target}
    POST_BUILD
    COMMENT
      "Staging ${_target}'s libs and handler into ${RV_STAGE_LIB_DIR} and ${RV_STAGE_BIN_DIR}, then converting crashpad_handler.exe to Windows GUI application"
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${_lib_dir} ${RV_STAGE_LIB_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${_bin_dir} ${RV_STAGE_BIN_DIR}
    COMMAND editbin /SUBSYSTEM:WINDOWS ${RV_STAGE_BIN_DIR}/${_crashpad_handler_name}
  )
  ADD_CUSTOM_TARGET(
    ${_target}-stage-target ALL
    DEPENDS ${RV_STAGE_BIN_DIR}/${_crashpad_handler_name}
  )
ELSE()
  ADD_CUSTOM_COMMAND(
    COMMENT "Staging ${_target}'s libs and handler into ${RV_STAGE_LIB_DIR} and ${RV_STAGE_BIN_DIR}"
    OUTPUT ${RV_STAGE_LIB_DIR}/${_crashpad_client_lib_name} ${RV_STAGE_BIN_DIR}/${_crashpad_handler_name}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${_lib_dir} ${RV_STAGE_LIB_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${_bin_dir} ${RV_STAGE_BIN_DIR}
    DEPENDS ${_target}
  )
  ADD_CUSTOM_TARGET(
    ${_target}-stage-target ALL
    DEPENDS ${RV_STAGE_LIB_DIR}/${_crashpad_client_lib_name}
    DEPENDS ${RV_STAGE_BIN_DIR}/${_crashpad_handler_name}
  )
ENDIF()

ADD_DEPENDENCIES(dependencies ${_target}-stage-target)

SET(RV_DEPS_CRASHPAD_VERSION
    ${_version}
    CACHE INTERNAL "" FORCE
)
