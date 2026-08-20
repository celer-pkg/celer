# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## SvgWidgets Module:
#####################################################################

qt_internal_add_module(SvgWidgets
    NO_PRIVATE_MODULE
    SOURCES
        qsvgwidget.cpp qsvgwidget.h
        qtsvgwidgetsglobal.h
    DEFINES
        QT_NO_CONTEXTLESS_CONNECT
        QT_NO_USING_NAMESPACE
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WidgetsPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Svg
        Qt::Widgets
)

qt_internal_extend_target(SvgWidgets CONDITION QT_FEATURE_graphicsview
    SOURCES
        qgraphicssvgitem.cpp qgraphicssvgitem.h
)

## Scopes:
#####################################################################

qt_internal_extend_target(SvgWidgets CONDITION MSVC AND (TEST_architecture_arch STREQUAL "i386")
    LINK_OPTIONS
        "/BASE:0x66000000"
)
