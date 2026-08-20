# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(Protobuf
    SOURCES
        protobuffieldpresencechecker_p.h
        protobufscalarserializers_p.h protobufscalarserializers_p.cpp
        protobufscalarjsonserializers_p.h
        qtprotobufglobal.h
        qabstractprotobufserializer.cpp qabstractprotobufserializer.h
        qprotobufdeserializerbase_p.h qprotobufdeserializerbase.cpp
        qprotobufjsonserializer.cpp qprotobufjsonserializer.h qprotobufjsonserializer_p.h
        qprotobuflazymessagepointer.h
        qprotobufmessage.cpp qprotobufmessage.h qprotobufmessage_p.h
        qprotobufobject.h
        qprotobufoneof.cpp qprotobufoneof.h
        qprotobufpropertyordering.cpp qprotobufpropertyordering.h
        qprotobufpropertyorderingbuilder_p.h
        qprotobufregistration.cpp qprotobufregistration.h qprotobufregistration_p.h
        qprotobufrepeatediterator.cpp qprotobufrepeatediterator.h
        qprotobufselfcheckiterator_p.h
        qprotobufserializer.cpp qprotobufserializer.h qprotobufserializer_p.h
        qtprotobufdefs_p.h
        qtprotobuflogging.cpp qtprotobuflogging_p.h
        qtprotobufserializerhelpers.cpp qtprotobufserializerhelpers_p.h
        qtprotobuftypes.cpp qtprotobuftypes.h
        qprotobufserializerbase_p.h qprotobufserializerbase.cpp
    EXTRA_CMAKE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/QtProtobufProperties.cmake.in"
    PUBLIC_LIBRARIES
        Qt::Core
    LIBRARIES
        Qt::CorePrivate
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
)

qt_internal_add_docs(Protobuf
    doc/qtprotobuf.qdocconf
)

# Suppress 'No relevant classes found. No output generated.' warning
set_source_files_properties(qprotobufobject.h PROPERTIES SKIP_AUTOMOC ON)

if(CMAKE_VERSION GREATER_EQUAL 3.27)
    set_target_properties(Protobuf PROPERTIES INTERFACE_AUTOMOC_MACRO_NAMES "Q_PROTOBUF_OBJECT")
endif()

# TODO: Remove this and fix it in qtbase's qt_internal_find_tool directly
if(QT_HOST_PATH AND Qt6::qtprotobufgen IN_LIST Qt6ProtobufTools_TARGETS
    AND NOT QT_FORCE_BUILD_TOOLS)
    qt_record_extra_qt_main_tools_package_dependency(Protobuf ProtobufTools "${PROJECT_VERSION}")
endif()
