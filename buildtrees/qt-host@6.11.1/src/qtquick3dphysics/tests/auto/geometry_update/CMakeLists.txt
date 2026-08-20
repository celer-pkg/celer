# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(PROJECT_NAME "test_auto_geometry_update")

qt_internal_add_test(${PROJECT_NAME}
    GUI
    QMLTEST
    SOURCES
        ../shared/util.h
        tst_geometry_update.cpp
    LIBRARIES
        Qt::Core
        Qt::Qml
        Qt::Gui
        Qt::Quick3D
    TESTDATA
        tst_geometry_update.qml
    BUILTIN_TESTDATA
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(${PROJECT_NAME})
endif()

qt_add_qml_module(${PROJECT_NAME}
    URI TileGeometry
    VERSION 1.0
    QML_FILES
        tst_geometry_update.qml
    SOURCES
        tilegeometry.cpp tilegeometry.h
    RESOURCE_PREFIX "/qt/qml"
    IMPORTS
        QtQuick3D
)
