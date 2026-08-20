# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from websockets.pro.

#####################################################################
## WebSockets Module:
#####################################################################

qt_internal_add_module(WebSockets
    SOURCES
        qdefaultmaskgenerator_p.cpp qdefaultmaskgenerator_p.h
        qmaskgenerator.cpp qmaskgenerator.h
        qwebsocket.cpp qwebsocket.h qwebsocket_p.cpp qwebsocket_p.h
        qwebsocketcorsauthenticator.cpp qwebsocketcorsauthenticator.h qwebsocketcorsauthenticator_p.h
        qwebsocketdataprocessor.cpp qwebsocketdataprocessor_p.h
        qwebsocketframe.cpp qwebsocketframe_p.h
        qwebsockethandshakeoptions.cpp qwebsockethandshakeoptions.h qwebsockethandshakeoptions_p.h
        qwebsockethandshakerequest.cpp qwebsockethandshakerequest_p.h
        qwebsockethandshakeresponse.cpp qwebsockethandshakeresponse_p.h
        qwebsocketprotocol.cpp qwebsocketprotocol.h qwebsocketprotocol_p.h
        qwebsockets_global.h
        qwebsocketserver.cpp qwebsocketserver.h qwebsocketserver_p.cpp qwebsocketserver_p.h
    DEFINES
        QT_NO_CONTEXTLESS_CONNECT
    LIBRARIES
        Qt::CorePrivate
        Qt::NetworkPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
)

#### Keys ignored in scope 1:.:.:websockets.pro:<TRUE>:
# OTHER_FILES = "doc/src/*.qdoc" "doc/snippets/*.cpp" "doc/qtwebsockets.qdocconf"

## Scopes:
#####################################################################

qt_internal_extend_target(WebSockets CONDITION WASM
    SOURCES
        qwebsocket_wasm_p.cpp
    LIBRARIES
        websocket.js
)

qt_internal_add_docs(WebSockets
    doc/qtwebsockets.qdocconf
)

