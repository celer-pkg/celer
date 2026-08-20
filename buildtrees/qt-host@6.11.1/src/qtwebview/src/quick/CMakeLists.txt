# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


if(TARGET Qt::WebEngineQuick)
    set(qmlDependency "QtWebEngine/auto")
endif()

qt_internal_add_qml_module(WebViewQuick
    URI "QtWebView"
    VERSION "${PROJECT_VERSION}"
    CLASS_NAME QWebViewQuickPlugin
    PLUGIN_TARGET qtwebviewquickplugin
    DEPENDENCIES
        QtQuick/auto
        ${qmlDependency}
    SOURCES
        qquickwebview.cpp qquickwebview_p.h
        qquickwebviewloadrequest.cpp qquickwebviewloadrequest_p.h
        qtwebviewquickglobal_p.h
        qquickwebviewsettings.cpp qquickwebviewsettings_p.h
    PUBLIC_LIBRARIES # TODO: QTBUG-86533 workaround, PUBLIC_LIBRARIES should be moved to LIBRARIES.
        Qt::Quick
        Qt::QmlModels
    LIBRARIES
        Qt::QuickPrivate
        Qt::WebViewPrivate
    PRIVATE_MODULE_INTERFACE
        Qt::QuickPrivate
)

 # TODO: QTBUG-86533 workaround, PUBLIC_LIBRARIES should be moved to LIBRARIES.
qt_internal_extend_target(WebViewQuick CONDITION TARGET Qt::OpenGL
    PUBLIC_LIBRARIES
        Qt::OpenGL
)
