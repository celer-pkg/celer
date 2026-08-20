# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_protobuf_module(ProtobufWellKnownTypes
    SOURCES
        qtprotobufwellknowntypesglobal.h
        qprotobufanysupport.cpp qprotobufanysupport.h
        qprotobufwellknowntypesjsonserializers_p.h qprotobufwellknowntypesjsonserializers.cpp
    PUBLIC_LIBRARIES
        Qt::Protobuf
    EXTRA_CMAKE_INCLUDES
        # Generated inside the qt_internal_add_protobuf_wellknown_types call
        "${QT_CMAKE_EXPORT_NAMESPACE}ProtobufWellKnownTypesExtras.cmake"
    LIBRARIES
        Qt::ProtobufPrivate
)

qt_internal_add_protobuf_wellknown_types(ProtobufWellKnownTypes
    TYPES
        any
        api
        duration
        empty
        field_mask
        source_context
        struct
        timestamp
        type
        wrappers
)
