# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QSvgIconPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QSvgIconPlugin
    OUTPUT_NAME qsvgicon
    PLUGIN_TYPE iconengines
    SOURCES
        main.cpp
        qsvgiconengine.cpp qsvgiconengine.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
        Qt::Svg
)
