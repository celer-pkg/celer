# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_protobuf_module(ProtobufQtCoreTypes
    SOURCES
        qtprotobufqtcoretypes.cpp
        qtprotobufqtcoretypes.h
        qtprotobufqtcoretypesglobal.h
        qtprotobufqttypescommon_p.h
    LIBRARIES
        Qt::Core
        Qt::Protobuf
        Qt::ProtobufPrivate
    PROTO_FILES
        QtCore/QtCore.proto
    PRIVATE_HEADER_FILTERS
        ".*\\.qpb\\.h"
)
