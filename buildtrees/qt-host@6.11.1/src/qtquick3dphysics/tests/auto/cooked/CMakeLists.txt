# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(PROJECT_NAME "test_auto_cooked")

qt_internal_add_test(${PROJECT_NAME}
    GUI
    QMLTEST
    SOURCES
        ../shared/util.h
        tst_cooked.cpp
    LIBRARIES
        Qt::Core
        Qt::Qml
    TESTDATA
        data/hf.cooked.hf
        data/tetrahedron.cooked.cvx
        data/tetrahedron.cooked.tri
        data/tst_cooked.qml
    BUILTIN_TESTDATA
)

if(QT_BUILD_STANDALONE_TESTS)
    qt_import_qml_plugins(${PROJECT_NAME})
endif()
