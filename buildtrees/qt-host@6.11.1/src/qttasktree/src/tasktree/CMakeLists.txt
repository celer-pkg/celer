# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## TaskTree Module:
#####################################################################

qt_internal_add_module(TaskTree
    NO_PRIVATE_MODULE
    SOURCES
        qbarriertask.cpp
        qbarriertask.h
        qconditional.cpp
        qconditional.h
        qtasktree.cpp
        qtasktree.h
        qtasktreerunner.cpp
        qtasktreerunner.h
        qttasktreeglobal.h
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CONTEXTLESS_CONNECT
        QT_NO_FOREACH
        QT_NO_USING_NAMESPACE
        QT_USE_NODISCARD_FILE_OPEN
    PUBLIC_LIBRARIES
        Qt::Core
    LIBRARIES
        Qt::CorePrivate
)

## Scopes:
#####################################################################

qt_internal_extend_target(TaskTree CONDITION QT_FEATURE_concurrent
    SOURCES
        qthreadfunctiontask.cpp
        qthreadfunctiontask.h
    PUBLIC_LIBRARIES
        Qt::Concurrent
)

qt_internal_extend_target(TaskTree CONDITION QT_FEATURE_network
    SOURCES
        qnetworkreplywrappertask.cpp
        qnetworkreplywrappertask.h
        qtcpsocketwrappertask.cpp
        qtcpsocketwrappertask.h
    PUBLIC_LIBRARIES
        Qt::Network
)

qt_internal_extend_target(TaskTree CONDITION QT_FEATURE_process
    SOURCES
        qprocesstask.cpp
        qprocesstask.h
)

if (NOT QT_FEATURE_process)
    set_source_files_properties(qprocesstask.h PROPERTIES SKIP_AUTOMOC TRUE)
endif()

qt_internal_add_docs(TaskTree
    doc/qttasktree.qdocconf
)
