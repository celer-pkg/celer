# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_unlit_lit
    GUI
    SOURCES
        rhitest.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3DPrivate
)

# Resources:
set_source_files_properties("../shared/teapot.mesh"
    PROPERTIES QT_RESOURCE_ALIAS "teapot.mesh"
)
set(rhitest_resource_files
    "../shared/teapot.mesh"
    "SomePointLight.qml"
    "qt_logo.png"
    "rhitest.qml"
)

qt_internal_add_resource(manual_test_unlit_lit "rhitest"
    PREFIX
        "/"
    FILES
        ${rhitest_resource_files}
)
