# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from qwebsocket.pro.

#####################################################################
## tst_qwebsocket Test:
#####################################################################

qt_internal_add_test(tst_qwebsocket
    SOURCES
        tst_qwebsocket.cpp
    LIBRARIES
        Qt::WebSockets
        Qt::TestPrivate
    BUNDLE_ANDROID_OPENSSL_LIBS
)

set(qwebsocketshared_resource_files
    "../shared/localhost.cert"
    "../shared/localhost.key"
)
qt_internal_add_resource(tst_qwebsocket "qwebsocketshared"
    PREFIX
        "/"
    BASE
        "../shared"
    FILES
        ${qwebsocketshared_resource_files}
)

#### Keys ignored in scope 1:.:.:qwebsocket.pro:<TRUE>:
# TEMPLATE = "app"
