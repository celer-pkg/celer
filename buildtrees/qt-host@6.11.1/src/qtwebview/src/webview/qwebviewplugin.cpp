// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qwebviewplugin_p.h"

QT_BEGIN_NAMESPACE

QWebViewPlugin::QWebViewPlugin(QObject *parent) : QObject(parent)
{

}

QWebViewPlugin::~QWebViewPlugin()
{

}

void QWebViewPlugin::prepare() const
{
    // Only called for plugins that has "RequiresInit" set to true in their plugin metadata.
}

QT_END_NAMESPACE

#include "moc_qwebviewplugin_p.cpp"
