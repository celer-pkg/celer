# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qnearfieldtagtype2 LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if (NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_qnearfieldtagtype2 Test:
#####################################################################

# Collect test data
list(APPEND test_data "nfcdata/Dynamic Empty Tag.nfc")
list(APPEND test_data "nfcdata/Empty Tag.nfc")

qt_internal_add_test(tst_qnearfieldtagtype2
    SOURCES
        ../nfccommons/qnearfieldtarget_emulator.cpp ../nfccommons/qnearfieldtarget_emulator_p.h
        ../nfccommons/targetemulator.cpp ../nfccommons/targetemulator_p.h
        ../nfccommons/qnearfieldtagtype1.cpp ../nfccommons/qnearfieldtagtype1_p.h
        ../nfccommons/qnearfieldtagtype2.cpp ../nfccommons/qnearfieldtagtype2_p.h
        ../nfccommons/qtlv.cpp ../nfccommons/qtlv_p.h
        tst_qnearfieldtagtype2.cpp
    DEFINES
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/nfcdata"
    INCLUDE_DIRECTORIES
        ../nfccommons
    LIBRARIES
        Qt::NfcPrivate
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:qnearfieldtagtype2.pro:<TRUE>:
# _REQUIREMENTS = "contains(QT_CONFIG, private_tests)"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qnearfieldtagtype2 CONDITION builtin_testdata
    DEFINES
        BUILTIN_TESTDATA
)
