# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## sensor_explorer Binary:
#####################################################################

qt_internal_add_manual_test(tst_manual_sensor_explorer_widgets
    GUI
    SOURCES
        explorer.cpp explorer.h explorer.ui
        main.cpp
    LIBRARIES
        Qt::Sensors
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
)
