# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause
if(QT_FEATURE_webview_webengine_plugin OR
    QT_FEATURE_webview_webview2_plugin OR
    QT_FEATURE_webview_android_plugin OR
    QT_FEATURE_webview_darwin_plugin OR
    QT_FEATURE_webview_winrt_plugin OR
    QT_FEATURE_webview_wasm_plugin)
    add_subdirectory(webview)
    add_subdirectory(util)
    add_subdirectory(qml)
endif()
