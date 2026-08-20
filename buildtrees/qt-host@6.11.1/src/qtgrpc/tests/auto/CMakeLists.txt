# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_FEATURE_qtprotobufgen OR QT_FEATURE_qtgrpcgen)
    add_subdirectory(protocplugintestcommon)
endif()

if(QT_FEATURE_qtprotobufgen)
    add_subdirectory(protobufgen)
endif()
if(QT_FEATURE_qtgrpcgen)
    add_subdirectory(grpcgen)
endif()

if(NOT QT_INTERNAL_GRPC_TEST_MESSAGE_LATENCY_MSEC)
    if (WIN32)
        set(QT_INTERNAL_GRPC_TEST_MESSAGE_LATENCY_MSEC 2000)
    else()
        set(QT_INTERNAL_GRPC_TEST_MESSAGE_LATENCY_MSEC 300)
    endif()
endif()

add_subdirectory(protobuf)
if(QT_FEATURE_grpc AND TARGET Qt6::qtprotobufgen AND TARGET Qt6::qtgrpcgen)
    add_subdirectory(grpc)
endif()

if(TARGET Qt6::qtprotobufgen AND QT_FEATURE_protobufquick AND TARGET Qt6::QuickTest)
    add_subdirectory(protobufqml)
endif()

if(TARGET Qt6::qtgrpcgen AND TARGET Qt6::GrpcQuick AND TARGET Qt6::QuickTest)
    add_subdirectory(grpcquick)
endif()

if(TARGET Qt6::ProtobufQtCoreTypes
        AND TARGET Qt6::ProtobufQtGuiTypes
        AND TARGET Qt6::qtprotobufgen)
    add_subdirectory(protobufqttypes)
endif()
