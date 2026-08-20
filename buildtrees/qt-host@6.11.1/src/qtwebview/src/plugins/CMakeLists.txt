# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(QT_FEATURE_webview_android_plugin)
    add_subdirectory(android)
endif()

if(QT_FEATURE_webview_darwin_plugin)
    add_subdirectory(darwin)
endif()

if(QT_FEATURE_webview_wasm_plugin)
    add_subdirectory(wasm)
endif()

if(QT_FEATURE_webview_webview2_plugin)
    add_subdirectory(windows)
endif()

if(QT_FEATURE_webview_webengine_plugin)
    add_subdirectory(webengine)
endif()
