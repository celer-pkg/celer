# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## QDarwinWebViewPlugin Plugin:
#####################################################################

qt_internal_find_apple_system_framework(FWWebKit WebKit)
qt_internal_find_apple_system_framework(FWAppKit AppKit)
qt_internal_find_apple_system_framework(FWUIKit UIKit)
qt_internal_find_apple_system_framework(FWFoundation Foundation)

qt_internal_add_plugin(QDarwinWebViewPlugin
    OUTPUT_NAME qtwebview_darwin
    PLUGIN_TYPE webview
    SOURCES
        qdarwinwebview.mm qdarwinwebview_p.h
        qdarwinwebviewplugin.cpp
    LIBRARIES
        ${FWFoundation}
        ${FWWebKit}
        Qt::Core
        Qt::Gui
        Qt::Quick
        Qt::WebViewPrivate
)

qt_internal_extend_target(QDarwinWebViewPlugin CONDITION MACOS
    LIBRARIES
        ${FWAppKit}
)

qt_internal_extend_target(QDarwinWebViewPlugin CONDITION IOS
    LIBRARIES
        ${FWUIKit}
)
