# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(WASM)
    return()
endif()

qt_internal_add_test(tst_scxmlcoutput_arguments
    SOURCES
        tst_scxmlcoutput.cpp
    LIBRARIES
        Qt::Scxml
)

# This testcase tests providing the namespace
qt6_add_statecharts(tst_scxmlcoutput_arguments
    ../shared/ids1.scxml
    OPTIONS --classname MyClass
)
