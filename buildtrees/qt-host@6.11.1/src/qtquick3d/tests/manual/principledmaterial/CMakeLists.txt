# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_principledmaterial
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3DPrivate
)

# Resources:
set(qml_resource_files
    "MaterialControl.qml"
    "main.qml"
)

qt_internal_add_resource(manual_test_principledmaterial "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
set(materials_resource_files
    "maps/OpenfootageNET_garage-1024.hdr"
    "maps/metallic/basemetal.astc"
    "maps/metallic/normalrough.astc"
)

qt_internal_add_resource(manual_test_principledmaterial "materials"
    PREFIX
        "/"
    FILES
        ${materials_resource_files}
)
