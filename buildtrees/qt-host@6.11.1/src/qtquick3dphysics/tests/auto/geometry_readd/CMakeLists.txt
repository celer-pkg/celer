# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(PROJECT_NAME "test_auto_geometry_readd")

qt_internal_add_test(${PROJECT_NAME}
    GUI
    QMLTEST
    SOURCES
        ../shared/util.h
        tst_geometry_readd.cpp
    LIBRARIES
        Qt::Core
        Qt::Qml
        Qt::Gui
        Qt::Quick3D
    TESTDATA
        tst_geometry_readd.qml
    BUILTIN_TESTDATA
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(${PROJECT_NAME})
endif()

qt_add_qml_module(${PROJECT_NAME}
    URI Geometry
    VERSION 1.0
    QML_FILES
        tst_geometry_readd.qml
    SOURCES
        examplegeometry.cpp examplegeometry.h
    RESOURCE_PREFIX "/qt/qml"
    IMPORTS
        QtQuick3D
)
