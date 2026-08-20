# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QWebEngineWebViewPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWebEngineWebViewPlugin
    OUTPUT_NAME qtwebview_webengine
    PLUGIN_TYPE webview
    SOURCES
        qwebenginewebview.cpp qwebenginewebview_p.h
        qwebenginewebviewplugin.cpp
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::WebEngineCorePrivate
        Qt::WebEngineQuickPrivate
        Qt::WebViewPrivate
        Qt::WebViewQuickPrivate
)
