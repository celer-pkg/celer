# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_add_qml_module(MyLib_Mock
    STATIC
    URI MyLib
    QML_FILES
        QmlObj.qml
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/B/MyLib
)
