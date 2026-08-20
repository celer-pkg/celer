# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qregexp.pro.

#####################################################################
## tst_qregexp Test:
#####################################################################

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qregexp LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qregexp
    SOURCES
        tst_qregexp.cpp
    LIBRARIES
        Qt::Core5Compat
)

# Resources:
set(qregexp_resource_files
    "data/qdatastream_4.9.bin"
    "data/qdatastream_5.0.bin"
)

qt_internal_add_resource(tst_qregexp "qregexp"
    PREFIX
        "/"
    FILES
        ${qregexp_resource_files}
)

