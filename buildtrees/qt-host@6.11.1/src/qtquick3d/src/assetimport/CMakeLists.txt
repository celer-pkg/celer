# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from assetimport.pro.

#####################################################################
## Quick3DAssetImport Module:
#####################################################################

qt_internal_add_module(Quick3DAssetImport
    PLUGIN_TYPES assetimporters
    SOURCES
        qtquick3dassetimportglobal_p.h
        qssgassetimporter_p.h
        qssgassetimporterfactory.cpp qssgassetimporterfactory_p.h
        qssgassetimporterplugin_p.h
        qssgassetimportmanager.cpp qssgassetimportmanager_p.h
    DEFINES
        QT_BUILD_QUICK3DASSETIMPORT_LIB
    LIBRARIES
        Qt::CorePrivate
        Qt::Quick3DUtilsPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Qml
        Qt::Quick3DUtils
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::Quick3DUtilsPrivate
)

#### Keys ignored in scope 1:.:.:assetimport.pro:<TRUE>:
# MODULE = "quick3dassetimport"
# OTHER_FILES = "options-schema.json"
