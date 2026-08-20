# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

add_subdirectory(axbase)
add_subdirectory(control)
add_subdirectory(container)

# We disable the headersclean check, since these header files don't belong to the ActiveQt module.
# Headersclean rules fail because ActiveQt module doesn't link the necessary libraries so their
# compile flags and include directories are missing. We cannot simply link libraries because the
# missing include directories belong to the AxServer and AxContainer modules which have conflicting
# object files and meaning and cannot be linked in the same time.
qt_internal_add_module(ActiveQt
    HEADER_MODULE
    MODULE_INCLUDE_NAME ActiveQt
    NO_HEADERSCLEAN_CHECK
    SOURCES
        axbase/qaxtypefunctions_p.h
        axbase/qaxutils_p.h
        container/qaxbase.h container/qaxbase_p.h
        container/qaxobject.h container/qaxobject_p.h
        container/qaxobjectinterface.h
        container/qaxscript.h
        container/qaxselect.h
        container/qaxwidget.h container/qaxwidget_p.h
        control/qaxaggregated.h
        control/qaxbindable.h
        control/qaxfactory.h
        control/qclassfactory_p.h
        shared/qaxtypes_p.h
    NO_GENERATE_CPP_EXPORTS
    QT_LICENSE_ID QT_COMMERCIAL_OR_BSD3
)
qt_internal_add_docs(ActiveQt
    doc/activeqt.qdocconf
)
