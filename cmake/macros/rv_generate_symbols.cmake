#
# Copyright (C) 2026  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#
# RV_GENERATE_SYMBOLS: Generate Breakpad symbols for a target (macOS and Linux)
#
# Usage: RV_GENERATE_SYMBOLS(TARGET <target>)
#
# Generates a Breakpad .sym file for the given target, organized in the standard Breakpad directory structure under stage/app/symbols/. On macOS, also generates
# a dSYM bundle. Only runs on macOS/Linux when Breakpad is available (RV_DEPS_BREAKPAD_VERSION).
#

FUNCTION(RV_GENERATE_SYMBOLS)
  SET(flags
      ""
  )
  SET(args
      ""
  )
  SET(listArgs
      TARGET
  )
  CMAKE_PARSE_ARGUMENTS(arg "${flags}" "${args}" "${listArgs}" ${ARGN})

  IF(NOT arg_TARGET)
    MESSAGE(FATAL_ERROR "RV_GENERATE_SYMBOLS: 'TARGET' parameter is required.")
  ENDIF()

  IF(NOT RV_DEPS_BREAKPAD_VERSION)
    RETURN()
  ENDIF()

  IF(NOT RV_TARGET_DARWIN
     AND NOT RV_TARGET_LINUX
  )
    RETURN()
  ENDIF()

  SET(_symbols_dir
      ${CMAKE_BINARY_DIR}/stage/app/symbols
  )
  SET(_sym_file
      ${CMAKE_CURRENT_BINARY_DIR}/${arg_TARGET}.sym
  )
  SET(_organize_script
      ${PROJECT_SOURCE_DIR}/src/bin/apps/rv/organize_symbols.sh
  )

  # Ensure Breakpad tools are built before generating symbols
  ADD_DEPENDENCIES(${arg_TARGET} RV_DEPS_BREAKPAD-stage-target)

  IF(RV_TARGET_DARWIN)
    # Generate dSYM bundle (macOS only)
    ADD_CUSTOM_COMMAND(
      TARGET ${arg_TARGET}
      POST_BUILD
      COMMENT "Generating dSYM for ${arg_TARGET}"
      COMMAND dsymutil $<TARGET_FILE:${arg_TARGET}> -o $<TARGET_FILE:${arg_TARGET}>.dSYM
      VERBATIM
    )
  ENDIF()

  # Convert to Breakpad .sym format using dump_syms Note: uses sh -c with positional parameters and VERBATIM so CMake properly escapes all arguments (including
  # the redirect >), avoiding the backslash-space-before-> issue that causes Ninja to create a redirect target with a leading space.
  ADD_CUSTOM_COMMAND(
    TARGET ${arg_TARGET}
    POST_BUILD
    COMMENT "Generating Breakpad symbols for ${arg_TARGET}"
    COMMAND sh -c "${RV_STAGE_BIN_DIR}/dump_syms \"$1\" > \"$2\"" -- $<TARGET_FILE:${arg_TARGET}> ${_sym_file} DEPENDS ${RV_STAGE_BIN_DIR}/dump_syms
    VERBATIM
  )

  # Organize .sym file in Breakpad directory structure (symbols/MODULE_NAME/MODULE_ID/)
  ADD_CUSTOM_COMMAND(
    TARGET ${arg_TARGET}
    POST_BUILD
    COMMENT "Organizing Breakpad symbols for ${arg_TARGET}"
    COMMAND bash ${_organize_script} ${_sym_file} ${_symbols_dir} DEPENDS ${_sym_file}
    VERBATIM
  )
ENDFUNCTION()
