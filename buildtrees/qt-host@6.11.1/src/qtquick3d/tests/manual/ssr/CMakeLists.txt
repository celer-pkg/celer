# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_manual_test(manual_test_ssr
    GUI
    SOURCES
        manual_test_ssr.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
        Qt::Quick3D
)

# Resources:
set(ssr_resource_files
    "ManualTestSSR.qml"
    "MovingRing.qml"
)

qt_internal_add_resource(manual_test_ssr "ssr"
    PREFIX
        "/"
    FILES
        ${ssr_resource_files}
)
