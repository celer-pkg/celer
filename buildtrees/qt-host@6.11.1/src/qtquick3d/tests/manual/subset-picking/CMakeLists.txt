# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_subset_picking
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

set(qml_resource_files
    "main.qml"
)

qt_internal_add_resource(manual_test_subset_picking "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
