# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#### Inputs

#### Libraries
if(WIN32)
    qt_find_package(WebView2 PROVIDED_TARGETS WebView2::WebView2 MODULE_NAME core QMAKE_LIB webview2)
endif()
#### Tests

#### Features

qt_feature("webview-webview2-plugin" PUBLIC
    LABEL "WebView2 (Windows only)"
    PURPOSE "Provides native Microsoft Edge WebView2 control as a plugin for Qt WebView."
    CONDITION WIN32 AND MSVC AND WebView2_FOUND
)
qt_feature("webview-webengine-plugin" PUBLIC
    LABEL "WebEngine"
    PURPOSE "Provides QtWebEngine based plugin for Qt WebView."
    CONDITION TARGET Qt::WebEngineCore AND TARGET Qt::WebEngineQuick
)
qt_feature("webview-android-plugin" PUBLIC
    LABEL "Android WebView (Android only)"
    PURPOSE "Provides Android WebView plugin for Qt WebView."
    CONDITION ANDROID
)
qt_feature("webview-darwin-plugin" PUBLIC
    LABEL "Darwin WebKit (MacOS and IOS only)"
    PURPOSE "Provides Darwin Webkit plugin for Qt WebView."
    CONDITION MACOS OR IOS
)
qt_feature("webview-wasm-plugin" PUBLIC
    LABEL "Wasm Webview (Web Assembly only)"
    PURPOSE "Provides Wasm WebView plugin for Qt WebView."
    CONDITION WASM
)

qt_configure_add_summary_section(NAME "Qt WebView plugins")
qt_configure_add_summary_entry(ARGS "webview-webengine-plugin")
qt_configure_add_summary_entry(ARGS "webview-webview2-plugin")
qt_configure_add_summary_entry(ARGS "webview-android-plugin")
qt_configure_add_summary_entry(ARGS "webview-darwin-plugin")
qt_configure_add_summary_entry(ARGS "webview-wasm-plugin")
qt_configure_end_summary_section()

qt_configure_add_report_entry(
    TYPE WARNING
    MESSAGE "No WebView2 SDK found, compiling QtWebView without WebView2 plugin.\n
    Please set WEBVIEW2_SDK_ROOT to point to WebView2 SDK directory."
    CONDITION WIN32 AND MSVC AND NOT QT_FEATURE_webview_webview2_plugin
)
