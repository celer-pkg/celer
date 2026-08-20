# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## StateMachine Module:
#####################################################################

qt_internal_include_in_repo_target_set(qtstatemachine)

qt_internal_add_module(StateMachine
    SOURCES
        qabstractstate.cpp qabstractstate.h qabstractstate_p.h
        qabstracttransition.cpp qabstracttransition.h qabstracttransition_p.h
        qfinalstate.cpp qfinalstate.h qfinalstate_p.h
        qhistorystate.cpp qhistorystate.h qhistorystate_p.h
        qsignaleventgenerator_p.h
        qsignaltransition.cpp qsignaltransition.h qsignaltransition_p.h
        qstate.cpp qstate.h qstate_p.h
        qstatemachine.cpp qstatemachine.h qstatemachine_p.h
        qstatemachineglobal.h qstatemachineglobal_p.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    LIBRARIES
        Qt::CorePrivate
        Qt::ScxmlGlobalPrivate
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::ScxmlGlobalPrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(StateMachine CONDITION QT_FEATURE_qeventtransition
    SOURCES
        gui/qbasickeyeventtransition.cpp gui/qbasickeyeventtransition_p.h
        gui/qbasicmouseeventtransition.cpp gui/qbasicmouseeventtransition_p.h
        gui/qkeyeventtransition.cpp gui/qkeyeventtransition.h
        gui/qmouseeventtransition.cpp gui/qmouseeventtransition.h
        qeventtransition.cpp qeventtransition.h qeventtransition_p.h
    LIBRARIES
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Gui
    PRIVATE_MODULE_INTERFACE
        Qt::GuiPrivate
)

qt_internal_add_docs(StateMachine
    doc/qtstatemachine.qdocconf
)
