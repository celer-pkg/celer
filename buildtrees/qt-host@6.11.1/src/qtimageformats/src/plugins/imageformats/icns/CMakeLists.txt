# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QICNSPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QICNSPlugin
    OUTPUT_NAME qicns
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qicnshandler.cpp qicnshandler_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    LIBRARIES
        Qt::Core
        Qt::Gui
)
