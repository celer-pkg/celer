# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(../../httpserver/httpserver.cmake)
include(../../util/util.cmake)

qt_internal_add_test(tst_qwebengineprofilebuilder
    SOURCES
        tst_qwebengineprofilebuilder.cpp
    LIBRARIES
        Qt::TestPrivate
        Qt::WebEngineWidgets
        Test::HttpServer
        Test::Util
)

qt_internal_add_resource(tst_qwebengineprofilebuilder "profilebuilder"
    PREFIX
        "/"
    FILES
        "resources/server.key"
        "resources/server.pem"
        "resources/ca.pem"
)
