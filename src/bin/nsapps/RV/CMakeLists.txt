#
# Copyright (C) 2022  Autodesk, Inc. All Rights Reserved.
#
# SPDX-License-Identifier: Apache-2.0
#

INCLUDE(cxx_defaults)
INCLUDE(CPack)

SET(_target
    "RV"
)

FIND_PACKAGE(
  ${RV_QT_PACKAGE_NAME}
  COMPONENTS Gui
  REQUIRED
)

SET(_sources
    main.cpp
)

ADD_EXECUTABLE(
  ${_target}
  ${_sources}
)

TARGET_INCLUDE_DIRECTORIES(
  ${_target}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
)

TARGET_COMPILE_OPTIONS(
  ${_target}
  PRIVATE -DGIT_HEAD=\"${RV_GIT_COMMIT_SHORT_HASH}\" -DUSE_SIDECARS=1 -DMAJOR_VERSION=${RV_MAJOR_VERSION} -DMINOR_VERSION=${RV_MINOR_VERSION}
          -DREVISION_NUMBER=${RV_REVISION_NUMBER} -DRELEASE_DESCRIPTION=\"${RV_RELEASE_DESCRIPTION}\"
)

#
# Platform common libraries
TARGET_LINK_LIBRARIES(
  ${_target}
  PRIVATE DarwinBundle
          IOproxy
          MovieProxy
          MovieSideCar
          OpenEXR::OpenEXR
          MovieFB
          MovieProcedural
          Mu
          MuGL
          MuIO
          MuTwkApp
          PyTwkApp
          Qt::Gui
          IPCore
          RvApp
          RvCommon
          RvPackage
          TwkQtCoreUtil
          TwkApp
          TwkDeploy
          TwkExc
          TwkFB
          TwkGLF
          TwkMovie
          TwkUtil
          arg
          stl_ext
          BDWGC::Gc
)

# Set source version in Mach-O binary for crash dump symbolication
IF(RV_TARGET_DARWIN)
  TARGET_LINK_OPTIONS(${_target} PRIVATE "-Wl,-source_version,${RV_MAJOR_VERSION}.${RV_MINOR_VERSION}.${RV_REVISION_NUMBER}")
ENDIF()

GET_PROPERTY(
  RV_THIRD_PARTY_CUSTOMIZATION GLOBAL
  PROPERTY "RV_THIRD_PARTY_CUSTOMIZATION"
)
IF(RV_THIRD_PARTY_CUSTOMIZATION)
  TARGET_COMPILE_OPTIONS(
    ${_target}
    PRIVATE "-DRV_THIRD_PARTY_CUSTOMIZATION=1"
  )
  TARGET_LINK_LIBRARIES(
    ${_target}
    PRIVATE ${RV_THIRD_PARTY_CUSTOMIZATION}
  )
ENDIF()

# Ensure Breakpad tools are built before generating symbols
IF(RV_DEPS_BREAKPAD_VERSION)
  ADD_DEPENDENCIES(${_target} RV_DEPS_BREAKPAD-stage-target)
ENDIF()

RV_STAGE(TYPE "MAIN_EXECUTABLE" TARGET ${_target})

# Generate dSYM and Breakpad symbols for crash dump symbolication
IF(RV_TARGET_DARWIN
   AND RV_DEPS_BREAKPAD_VERSION
)
  # Find the parent directory of RV.app (stage/app/)
  GET_FILENAME_COMPONENT(_app_parent_dir ${CMAKE_BINARY_DIR}/stage/app ABSOLUTE)

  # First generate dSYM
  ADD_CUSTOM_COMMAND(
    TARGET ${_target}
    POST_BUILD
    COMMENT "Generating dSYM for ${_target}"
    COMMAND dsymutil $<TARGET_FILE:${_target}> -o ${_app_parent_dir}/${_target}.app.dSYM
    VERBATIM
  )

  # Then convert to Breakpad .sym format
  ADD_CUSTOM_COMMAND(
    TARGET ${_target}
    POST_BUILD
    COMMENT "Generating Breakpad symbols for ${_target}"
    COMMAND ${RV_STAGE_BIN_DIR}/dump_syms $<TARGET_FILE:${_target}> > ${CMAKE_CURRENT_BINARY_DIR}/rv.sym DEPENDS ${RV_STAGE_BIN_DIR}/dump_syms
    VERBATIM
  )

  # Organize .sym file in Breakpad directory structure (symbols/MODULE_NAME/MODULE_ID/)
  ADD_CUSTOM_COMMAND(
    TARGET ${_target}
    POST_BUILD
    COMMENT "Organizing Breakpad symbols for ${_target}"
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/organize_symbols.sh ${CMAKE_CURRENT_BINARY_DIR}/rv.sym ${_app_parent_dir}/symbols DEPENDS
            ${CMAKE_CURRENT_BINARY_DIR}/rv.sym
    VERBATIM
  )
ENDIF()

FILE(GLOB _icns_files *.icns)
FOREACH(
  _icns_file
  ${_icns_files}
)
  CONFIGURE_FILE(${_icns_file} ${RV_STAGE_RESOURCES_DIR} COPYONLY)
ENDFOREACH()

# Install symbolication helper scripts (macOS/Linux only - nsapps directory)
IF(RV_TARGET_DARWIN
   AND RV_DEPS_BREAKPAD_VERSION
)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/symbolicate_crash.sh ${RV_STAGE_BIN_DIR}/symbolicate_crash.sh COPYONLY)
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/locate_crash_source.sh ${RV_STAGE_BIN_DIR}/locate_crash_source.sh COPYONLY)
ENDIF()
