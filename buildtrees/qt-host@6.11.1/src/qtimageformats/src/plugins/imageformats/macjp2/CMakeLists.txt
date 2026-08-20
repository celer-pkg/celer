# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QMacJp2Plugin Plugin:
#####################################################################

qt_internal_add_plugin(QMacJp2Plugin
    OUTPUT_NAME qmacjp2
    PLUGIN_TYPE imageformats
    SOURCES
        ../shared/qiiofhelpers.mm ../shared/qiiofhelpers_p.h
        main.cpp
        qmacjp2handler.cpp qmacjp2handler.h
    INCLUDE_DIRECTORIES
        ../shared
    DEFINES
        NS_IIOF_HELPERS=QMacJp2
    LIBRARIES
        ${FWCoreFoundation}
        ${FWCoreGraphics}
        ${FWImageIO}
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
)
