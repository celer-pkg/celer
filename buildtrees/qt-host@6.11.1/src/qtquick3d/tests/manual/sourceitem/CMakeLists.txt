# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_sourceitem
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

# Resources:
set(qml_resource_files
    "main.qml"
    "qt_logo.png"
    "qt_logo_rect.png"
)

qt_internal_add_resource(manual_test_sourceitem "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
