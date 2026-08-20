# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

file(GLOB_RECURSE test_data_glob
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    *)
list(APPEND test_data ${test_data_glob})

qt_internal_add_test(tst_qmltest_datavis
    QMLTEST
    SOURCES
        tst_qmltest.cpp
    LIBRARIES
        Qt::Gui
        Qt::GuiPrivate
    TESTDATA ${test_data}
)

set(qmltest_resource_files
    "customitem.obj"
    "customtexture.jpg"
)

qt_internal_add_resource(tst_qmltest_datavis "qmltest"
    PREFIX
        "/"
    FILES
        ${qmltest_resource_files}
)
