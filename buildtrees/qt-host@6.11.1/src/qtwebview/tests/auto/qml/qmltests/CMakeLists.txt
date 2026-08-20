# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Qml QuickTest)

qt_internal_add_test(tst_webview_qmltests
    QMLTEST
    SOURCES
        tst_qmltests.cpp
    LIBRARIES
        Qt::Qml
        Qt::WebView
        Qt::WebViewQuick
    TESTDATA
        data/tst_create_destroy.qml
)
