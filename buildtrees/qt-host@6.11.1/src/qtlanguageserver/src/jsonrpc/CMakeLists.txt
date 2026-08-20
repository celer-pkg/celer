# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## JsonRpc Module:
#####################################################################

qt_internal_add_module(JsonRpcPrivate
    INTERNAL_MODULE
    STATIC
    SOURCES
        qhttpmessagestreamparser_p.h qhttpmessagestreamparser.cpp
        qjsonrpcprotocol.cpp qjsonrpcprotocol_p.h qjsonrpcprotocol_p_p.h
        qjsonrpctransport_p.h
        qtjsonrpcglobal.h
        qjsontypedrpc_p.h qjsontypedrpc.cpp
        qtypedjson_p.h qtypedjson.cpp
    DEFINES
        QT_BUILD_JSONRPC_LIB
        QT_NO_CONTEXTLESS_CONNECT
    PUBLIC_LIBRARIES
        Qt::Core
)
