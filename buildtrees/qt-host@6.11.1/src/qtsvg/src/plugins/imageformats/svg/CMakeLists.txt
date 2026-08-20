# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QSvgPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QSvgPlugin
    OUTPUT_NAME qsvg
    PLUGIN_TYPE imageformats
    SOURCES
        main.cpp
        qsvgiohandler.cpp qsvgiohandler.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::SvgPrivate
)
