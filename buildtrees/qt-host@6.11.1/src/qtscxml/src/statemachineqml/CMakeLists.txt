# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_include_in_repo_target_set(qtstatemachineqml)

qt_internal_add_qml_module(StateMachineQml
    URI "QtQml.StateMachine"
    VERSION "${PROJECT_VERSION}"
    PLUGIN_TARGET qtqmlstatemachine
    CLASS_NAME QtQmlStateMachinePlugin
    SOURCES
        qstatemachineqmlglobals_p.h
        finalstate_p.h finalstate.cpp
        signaltransition_p.h signaltransition.cpp
        state_p.h state.cpp
        statemachine_p.h statemachine.cpp
        timeouttransition_p.h timeouttransition.cpp
        childrenprivate_p.h
        statemachineforeign_p.h
    DEFINES
        QT_BUILD_STATEMACHINEQML_LIB
    DEPENDENCIES
        QtQml/auto
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Qml
        Qt::StateMachine
    LIBRARIES
        Qt::QmlPrivate
)
