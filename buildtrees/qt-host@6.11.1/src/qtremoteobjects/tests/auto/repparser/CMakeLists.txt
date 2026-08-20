# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_parser Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_parser LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

set(REPPARSER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/repparser")

qt_internal_add_test(tst_parser
    SOURCES
        tst_parser.cpp
    INCLUDE_DIRECTORIES
        ${REPPARSER_DIR}
    LIBRARIES
        Qt::CorePrivate
)

# QLALR Grammars:
qt_process_qlalr(
    tst_parser
    ${REPPARSER_DIR}/parser.g
    "--exit-on-warn"
)

## Scopes:
#####################################################################

qt_internal_extend_target(tst_parser CONDITION MSVC
    COMPILE_OPTIONS
        /wd4129
)
