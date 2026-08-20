# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(PROJECT_NAME "test_auto_heightfield_readd")

qt_internal_add_test(${PROJECT_NAME}
    GUI
    QMLTEST
    SOURCES
        ../shared/util.h
        tst_heightfield_readd.cpp
    LIBRARIES
        Qt::Core
        Qt::Qml
        Qt::Gui
        Qt::Quick3D
        Qt::Quick3DPhysics
    TESTDATA
        tst_heightfield_readd.qml
        data/hf.png
    BUILTIN_TESTDATA
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(${PROJECT_NAME})
endif()
