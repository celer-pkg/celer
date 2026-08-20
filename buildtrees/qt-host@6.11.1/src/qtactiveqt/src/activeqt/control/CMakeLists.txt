# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## AxServer Module:
#####################################################################

qt_internal_add_module(AxServer
    STATIC
    QMAKE_MODULE_CONFIG idcidl force_import_plugins
    SOURCES
        ../shared/qaxtypes.cpp ../shared/qaxtypes_p.h
        qaxaggregated.cpp qaxaggregated.h
        qaxbindable.cpp qaxbindable.h
        qaxfactory.cpp qaxfactory.h
        qaxmain.cpp
        qaxserver.cpp
        qaxserverbase.cpp
        qaxserverdll.cpp
        qaxservermain.cpp
        qclassfactory_p.h
    DEFINES
        QAX_SERVER
    PUBLIC_DEFINES
        QAXSERVER
    LIBRARIES
        Qt::AxBasePrivate
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WidgetsPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Widgets
        shell32
    NO_GENERATE_CPP_EXPORTS
    QT_LICENSE_ID QT_COMMERCIAL_OR_BSD3
)

set_source_files_properties(qaxservermain.cpp # define main()
                            qaxmain.cpp
                            PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

## Scopes:
#####################################################################

qt_internal_extend_target(AxServer CONDITION MINGW
    DEFINES
        QT_NEEDS_QMAIN
)
