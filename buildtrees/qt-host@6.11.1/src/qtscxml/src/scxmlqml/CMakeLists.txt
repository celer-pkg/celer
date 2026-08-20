# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_include_in_repo_target_set(qtscxmlqml)

qt_internal_add_qml_module(ScxmlQml
    URI "QtScxml"
    VERSION "${PROJECT_VERSION}"
    PLUGIN_TARGET declarative_scxml
    CLASS_NAME QScxmlStateMachinePlugin
    SOURCES
        eventconnection.cpp eventconnection_p.h
        invokedservices.cpp invokedservices_p.h
        statemachineextended.cpp statemachineextended_p.h
        statemachineloader.cpp statemachineloader_p.h
        qscxmlqmlglobals_p.h
    DEFINES
        QT_BUILD_SCXMLQML_LIB
    DEPENDENCIES
        QtQml/auto
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Qml
        Qt::Scxml
    LIBRARIES
        Qt::CorePrivate
)
