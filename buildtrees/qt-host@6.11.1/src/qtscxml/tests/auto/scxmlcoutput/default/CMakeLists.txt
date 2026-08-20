# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(WASM)
    return()
endif()

qt_internal_add_test(tst_scxmlcoutput_default
    SOURCES
        tst_scxmlcoutput.cpp
    LIBRARIES
        Qt::Scxml
)

# This testcase tests that the default code generation,
# ie. no options provided for the qt6_add_statecharts
qt6_add_statecharts(tst_scxmlcoutput_default
    ../shared/ids1.scxml
)
