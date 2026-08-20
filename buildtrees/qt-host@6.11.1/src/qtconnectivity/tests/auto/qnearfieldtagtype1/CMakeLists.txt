# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qnearfieldtagtype1 LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

if (NOT QT_FEATURE_private_tests)
    return()
endif()

#####################################################################
## tst_qnearfieldtagtype1 Test:
#####################################################################

# Collect test data
list(APPEND test_data "../nfcdata/Qt Labs Website Tag Type1.nfc")
list(APPEND test_data "../nfcdata/Qt Website Tag Type1.nfc")

qt_internal_add_test(tst_qnearfieldtagtype1
    SOURCES
        ../nfccommons/qnearfieldtarget_emulator.cpp ../nfccommons/qnearfieldtarget_emulator_p.h
        ../nfccommons/targetemulator.cpp ../nfccommons/targetemulator_p.h
        ../nfccommons/qnearfieldtagtype1.cpp ../nfccommons/qnearfieldtagtype1_p.h
        ../nfccommons/qnearfieldtagtype2.cpp ../nfccommons/qnearfieldtagtype2_p.h
        ../nfccommons/qtlv.cpp ../nfccommons/qtlv_p.h
        tst_qnearfieldtagtype1.cpp
    DEFINES
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/../nfcdata"
    INCLUDE_DIRECTORIES
        ../nfccommons
    LIBRARIES
        Qt::NfcPrivate
    TESTDATA ${test_data}
)

#### Keys ignored in scope 1:.:.:qnearfieldtagtype1.pro:<TRUE>:
# _REQUIREMENTS = "contains(QT_CONFIG, private_tests)"

## Scopes:
#####################################################################

qt_internal_extend_target(tst_qnearfieldtagtype1 CONDITION builtin_testdata
    DEFINES
        BUILTIN_TESTDATA
)
