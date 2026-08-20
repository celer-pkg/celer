# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_qml_module(GrpcQuick
    URI QtGrpc
    VERSION ${CMAKE_PROJECT_VERSION}
    PLUGIN_TARGET grpcquickplugin
    CLASS_NAME GrpcQuickPlugin
    SOURCES
        qqmlabstractgrpcchannel.h qqmlabstractgrpcchannel_p.h qqmlabstractgrpcchannel.cpp
        qqmlgrpcnamespace_p.h
        qqmlgrpchttp2channel_p.h qqmlgrpchttp2channel.cpp
        qqmlgrpcchanneloptions_p.h qqmlgrpcchanneloptions.cpp
        qqmlgrpccalloptions.h qqmlgrpccalloptions.cpp
        qqmlgrpcmetadata_p.h qqmlgrpcmetadata.cpp
        qqmlgrpcfunctionalhandlers.h qqmlgrpcfunctionalhandlers.cpp
        qtgrpcquicklogging_p.h qtgrpcquicklogging.cpp
        qtqmlgrpcstreamsender.h qtqmlgrpcstreamsender.cpp
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Grpc
    PUBLIC_LIBRARIES
        Qt::Quick
    QT_LICENSE_ID QT_COMMERCIAL_OR_GPL3
)

qt_internal_extend_target(GrpcQuick CONDITION QT_FEATURE_qml_ssl
    LIBRARIES
        Qt::QmlNetwork
        Qt::QmlNetworkPrivate
)
