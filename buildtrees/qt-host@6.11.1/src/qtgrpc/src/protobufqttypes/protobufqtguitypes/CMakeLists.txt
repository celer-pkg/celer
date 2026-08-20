# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_protobuf_module(ProtobufQtGuiTypes
    SOURCES
        qtprotobufqtguitypes.cpp
        qtprotobufqtguitypes.h
        qtprotobufqtguitypesglobal.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::Protobuf
        Qt::ProtobufPrivate
        Qt::ProtobufQtCoreTypesPrivate
    PUBLIC_LIBRARIES
        Qt::ProtobufQtCoreTypes # TODO: QTBUG-86533 workaround
    PROTO_FILES
        QtGui/QtGui.proto
    PRIVATE_HEADER_FILTERS
        ".*\\.qpb\\.h"
)
