// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtwebviewfunctions.h"

#include "qwebviewfactory_p.h"
#include "qwebviewplugin_p.h"

#include <QtCore/QCoreApplication>

QT_BEGIN_NAMESPACE

/*!
    \namespace QtWebView
    \inmodule QtWebView
    \brief The QtWebView namespace provides functions that makes it easier to set-up and use the WebView.
    \inheaderfile QtWebView
*/

// This is a separate function so we can be sure that in non-static cases it can be registered
// as a pre hook for QCoreApplication, ensuring this is called after the plugin paths have
// been set to their defaults. For static builds then this will be called explicitly when
// QtWebView::initialize() is called by the application

static void initializeImpl()
{
    if (QWebViewFactory::requiresExtraInitializationSteps()) {
        // There might be plugins available, but their dependencies might not be met,
        // so make sure we have a valid plugin before using it.
        // Note: A warning will be printed later if we're unable to load the plugin.
        QWebViewPlugin *plugin = QWebViewFactory::getPlugin();
        if (plugin)
            plugin->prepare();
    }
}

#ifndef QT_STATIC
Q_COREAPP_STARTUP_FUNCTION(initializeImpl);
#endif

/*!
    \fn void QtWebView::initialize()
    \keyword qtwebview-initialize

    This function initializes resources or sets options that are required by the different back-ends.

    \note The \c initialize() function needs to be called immediately before the QGuiApplication
    instance is created.
 */

void QtWebView::initialize()
{
#ifdef QT_STATIC
    initializeImpl();
#endif
}

QT_END_NAMESPACE
