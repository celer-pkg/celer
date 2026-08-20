# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Qt6 COMPONENTS Widgets QuickWidgets)

qt_internal_add_manual_test(manual_tst_inquickwidget
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Widgets
        Qt::QuickWidgets
        Qt::WebView
)

# Resources:
set(qml_resource_files
    "main.qml"
)

qt_internal_add_resource(manual_tst_inquickwidget "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)
