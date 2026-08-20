# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## WebView Module:
#####################################################################

qt_internal_add_module(WebView
    PLUGIN_TYPES webview
    DEPENDENCIES
        QtQuick/auto
    SOURCES
        qtwebviewfunctions.cpp qtwebviewfunctions.h
        qwebview.cpp qwebview.h qwebview_p.h
        qwebview_global.h
        qwebviewfactory.cpp qwebviewfactory_p.h
        qwebviewloadinginfo.cpp qwebviewloadinginfo.h
        qwebviewsettings.cpp qwebviewsettings.h
        qwebviewplugin.cpp qwebviewplugin_p.h
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::GuiPrivate
    NO_GENERATE_CPP_EXPORTS
)

if(ANDROID)
    set_property(TARGET WebView APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/Qt${QtWebView_VERSION_MAJOR}AndroidWebView.jar
    )
    set_property(TARGET WebView APPEND PROPERTY QT_ANDROID_LIB_DEPENDENCIES
        ${INSTALL_PLUGINSDIR}/webview/libplugins_webview_qtwebview_android.so
    )
    qt_internal_add_android_permission(WebView
        NAME android.permission.ACCESS_FINE_LOCATION
    )
endif()
qt_internal_add_docs(WebView
    doc/qtwebview.qdocconf
)
