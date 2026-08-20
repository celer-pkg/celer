# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from positioning_backend.pro.

#####################################################################
## posbackendtesting Binary:
#####################################################################

qt_internal_add_executable(posbackendtesting
    GUI
    SOURCES
        logwidget.cpp logwidget.h
        main.cpp
        widget.cpp widget.h widget.ui
    LIBRARIES
        Qt::Gui
        Qt::Positioning
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)

#### Keys ignored in scope 1:.:.:positioning_backend.pro:<TRUE>:
# TEMPLATE = "app"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:positioning_backend.pro:WINRT:
# WINRT_MANIFEST.capabilities_device = "location"
