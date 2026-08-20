# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_offscreenrendermode
    GUI
    SOURCES
        offscreenrendermode.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3DPrivate
)

# Resources:
set_source_files_properties("../shared/teapot.mesh"
    PROPERTIES QT_RESOURCE_ALIAS "teapot.mesh"
)
set(offscreenrendermode_resource_files
    "../shared/teapot.mesh"
    "SomePointLight.qml"
    "offscreenrendermode.qml"
    "qt_logo.png"
)

qt_internal_add_resource(manual_test_offscreenrendermode "offscreenrendermode"
    PREFIX
        "/"
    FILES
        ${offscreenrendermode_resource_files}
)
