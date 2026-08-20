# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(QuickLayouts
    URI "QtQuick.Layouts"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    PLUGIN_TARGET qquicklayoutsplugin
    CLASS_NAME QtQuickLayoutsPlugin
    DEPENDENCIES
        QtQuick/auto
    SOURCES
        qquickgridlayoutengine.cpp qquickgridlayoutengine_p.h
        qquicklayout.cpp qquicklayout_p.h
        qquicklayoutstyleinfo.cpp qquicklayoutstyleinfo_p.h
        qquicklinearlayout.cpp qquicklinearlayout_p.h
        qquickstacklayout.cpp qquickstacklayout_p.h
        qquicklayoutitemproxy.cpp qquicklayoutitemproxy_p.h
        qquicklayoutglobal_p.h
    DEFINES
        QT_BUILD_QUICKLAYOUTS_LIB
    LIBRARIES
        Qt::GuiPrivate
        Qt::QuickPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Qml
    )

qt_internal_extend_target(QuickLayouts CONDITION NOT INTEGRITY AND NOT VXWORKS
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty
    SOURCES
        # Qt flexbox layout (uses inernally yoga engine to layout quick items)
        qquickflexboxlayout.cpp qquickflexboxlayout_p.h
        qquickflexboxlayoutengine.cpp qquickflexboxlayoutengine_p.h
        qquickflexboxlayoutitem_p.h qquickflexboxlayoutitem.cpp
        # Yoga library source
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/Yoga.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/event/event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/YGValue.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/YGStyle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/YGEnums.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/YGNodePrint.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/YGNode.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/YGLayout.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/YGEnums.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/YGConfig.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/Utils.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga/log.cpp
    ATTRIBUTION_FILE_DIR_PATHS
        "${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty/yoga"
    )
