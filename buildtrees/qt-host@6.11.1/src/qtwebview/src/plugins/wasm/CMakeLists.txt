# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QWasmWebViewPlugin Plugin:
#####################################################################

qt_internal_add_plugin(QWasmWebViewPlugin
    OUTPUT_NAME qtwebview_wasm
    PLUGIN_TYPE webview
    SOURCES
        qwasmwebview.cpp qwasmwebview_p.h
        qwasmwebviewplugin.cpp
    LIBRARIES
        Qt::CorePrivate
        Qt::Core
        Qt::Gui
        Qt::WebViewPrivate
)
