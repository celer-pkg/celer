# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QTgaPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QTgaPlugin
    OUTPUT_NAME qtga
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qtgafile.cpp qtgafile.h
        qtgahandler.cpp qtgahandler.h
    LIBRARIES
        Qt::Core
        Qt::Gui
)
