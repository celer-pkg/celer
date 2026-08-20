# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(WASM)
    return()
endif()

qt_internal_add_test(tst_scxmlcoutput_path
    SOURCES
        tst_scxmlcoutput.cpp
    LIBRARIES
        Qt::Scxml
)

# This testcase tests setting the output path
set(outputPath "${CMAKE_CURRENT_BINARY_DIR}/thechosenpath")
# Create the path first relative to the build directory, otherwise the
# code generation's output might fail on some platforms.
file(MAKE_DIRECTORY "${outputPath}")

qt6_add_statecharts(tst_scxmlcoutput_path
    ../shared/ids1.scxml
    OUTPUT_DIRECTORY "${outputPath}"
)
