# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS
  Widgets QuickWidgets
)

qt_internal_add_benchmark(mapitems_framecount
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Widgets
        Qt::Quick
        Qt::QuickWidgets
)

# Resources:
set(qml_resource_files
    "circles.qml"
    "rectangles.qml"
    "polylines.qml"
    "polygons.qml"
)

qt_internal_add_resource(mapitems_framecount "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
