# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QWbmpPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWbmpPlugin
    OUTPUT_NAME qwbmp
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qwbmphandler.cpp qwbmphandler_p.h
    LIBRARIES
        Qt::Core
        Qt::Gui
)
