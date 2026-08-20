# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_dynamicenvmap
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

set(dynamicenvmap_resource_files
    "main.qml"
    "InvertedSphere.mesh"
    "00455_OpenfootageNET_field_low_resized.jpg"
)

qt6_add_resources(manual_test_dynamicenvmap "dynamicenvmap"
    PREFIX
        "/"
    FILES
        ${dynamicenvmap_resource_files}
)
