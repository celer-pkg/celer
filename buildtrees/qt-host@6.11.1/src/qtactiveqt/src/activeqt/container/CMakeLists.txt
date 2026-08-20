# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## AxContainer Module:
#####################################################################

qt_internal_add_module(AxContainer
    STATIC
    QMAKE_MODULE_CONFIG dumpcpp
    SOURCES
        ../control/qaxaggregated.h
        ../shared/qaxtypes.cpp ../shared/qaxtypes_p.h
        qaxbase.cpp qaxbase.h qaxbase_p.h
        qaxdump.cpp
        qaxobject.cpp qaxobject.h qaxobject_p.h
        qaxobjectinterface.h
        qaxscript.cpp qaxscript.h
        qaxscriptwrapper.cpp
        qaxselect.cpp qaxselect.h qaxselect.ui
        qaxwidget.cpp qaxwidget.h qaxwidget_p.h
    LIBRARIES
        Qt::AxBasePrivate
        Qt::CorePrivate
        Qt::GuiPrivate
        Qt::WidgetsPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Widgets
    ENABLE_AUTOGEN_TOOLS
        uic
    NO_GENERATE_CPP_EXPORTS
    QT_LICENSE_ID QT_COMMERCIAL_OR_BSD3
)

set_source_files_properties(qaxbase.cpp # defines QT_CHECK_STATE
                            qaxscriptwrapper.cpp # references qAxFactory()
                            PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

qt_internal_add_sync_header_dependencies(AxContainer AxServer)
