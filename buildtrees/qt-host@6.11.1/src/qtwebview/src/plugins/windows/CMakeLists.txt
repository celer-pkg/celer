# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QWebview2Webview Plugin:
#####################################################################

qt_internal_add_plugin(QWebView2WebViewPlugin
    OUTPUT_NAME qtwebview_webview2
    PLUGIN_TYPE webview
    SOURCES
        qwebview2webview.cpp qwebview2webview_p.h
        qwebview2webviewplugin.cpp
    INCLUDE_DIRECTORIES
        ${WEBVIEW2_INCLUDE_DIR}
    LIBRARIES
        urlmon
        Qt::Core
        Qt::Gui
        Qt::WebViewPrivate
        WebView2::WebView2
    NO_PCH_SOURCES
        "qwebview2webview.cpp"
)
