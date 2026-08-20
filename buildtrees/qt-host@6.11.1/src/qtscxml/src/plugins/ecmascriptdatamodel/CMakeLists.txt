# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## EcmaScript SCXML DataModel Plugin:
#####################################################################

qt_internal_include_in_repo_target_set(qtscxmlqml)

qt_internal_add_plugin(QScxmlEcmaScriptDataModelPlugin
    OUTPUT_NAME qscxmlecmascriptdatamodel
    PLUGIN_TYPE scxmldatamodel
    SOURCES
        qscxmlecmascriptdatamodelplugin.cpp qscxmlecmascriptdatamodelplugin_p.h
        qscxmlecmascriptdatamodel_p.h qscxmlecmascriptdatamodel.cpp
        qscxmlecmascriptplatformproperties.cpp qscxmlecmascriptplatformproperties_p.h
    LIBRARIES
        Qt::Core
        Qt::Scxml
        Qt::Qml
    INCLUDE_DIRECTORIES
        $<TARGET_PROPERTY:Qt::QmlPrivate,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Qt::Scxml,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Qt::ScxmlPrivate,INTERFACE_INCLUDE_DIRECTORIES>
)

# OTHER_FILES = "ecmascriptdatamodel_plugin.json"
