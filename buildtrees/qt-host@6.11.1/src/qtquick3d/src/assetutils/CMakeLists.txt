# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(Quick3DAssetUtils
    URI "QtQuick3D.AssetUtils"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    CLASS_NAME QtQuick3DAssetUtilsPlugin
    PLUGIN_TARGET qtquick3dassetutilsplugin
    DEPENDENCIES
        QtQuick3D/auto
    SOURCES
        qtquick3dassetutilsglobal_p.h
        qssgscenedesc.cpp qssgscenedesc_p.h
        qssgqmlutilities.cpp qssgqmlutilities_p.h
        qssgsceneedit.cpp qssgsceneedit_p.h
        qssgrtutilities.cpp qssgrtutilities_p.h
        qquick3druntimeloader.cpp qquick3druntimeloader_p.h
    DEFINES
        QT_BUILD_QUICK3DASSETUTILS_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Quick3DAssetImportPrivate
        Qt::Quick3DRuntimeRenderPrivate
        Qt::Quick3DPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Quick
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::Quick3DRuntimeRenderPrivate
        Qt::Quick3DPrivate
        Qt::Quick3DAssetImportPrivate
)

qt_internal_extend_target(Quick3DAssetUtils CONDITION TARGET Qt::QuickTimeline
    LIBRARIES
        Qt::QuickTimelinePrivate
    PRIVATE_MODULE_INTERFACE
        Qt::QuickTimelinePrivate
    DEFINES
        QT_QUICK3D_ENABLE_RT_ANIMATIONS
)

if(QT_FEATURE_quick_designer AND QT_BUILD_SHARED_LIBS) # special case handle unconverted static
    add_subdirectory(designer)
endif()
