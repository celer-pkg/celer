# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QMacHeifPlugin Plugin:
#####################################################################

qt_internal_find_apple_system_framework(FWCoreFoundation
    CoreFoundation)
qt_internal_find_apple_system_framework(FWCoreGraphics CoreGraphics)
qt_internal_find_apple_system_framework(FWImageIO ImageIO)

qt_internal_add_plugin(QMacHeifPlugin
    OUTPUT_NAME qmacheif
    PLUGIN_TYPE imageformats
    SOURCES
        ../shared/qiiofhelpers.mm ../shared/qiiofhelpers_p.h
        main.cpp
        qmacheifhandler.cpp qmacheifhandler.h
    INCLUDE_DIRECTORIES
        ../shared
    DEFINES
        NS_IIOF_HELPERS=QMacHeif
    LIBRARIES
        ${FWCoreFoundation}
        ${FWCoreGraphics}
        ${FWImageIO}
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::GuiPrivate
)
