# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(PROJECT_NAME "test_auto_multiscene")

qt_internal_add_test(${PROJECT_NAME}
    GUI
    QMLTEST
    SOURCES
        ../shared/util.h
        tst_multiscene.cpp
    LIBRARIES
        Qt::Core
        Qt::Qml
    TESTDATA
        tst_multiscene.qml
        ImpellerScene.qml
    BUILTIN_TESTDATA
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(${PROJECT_NAME})
endif()
