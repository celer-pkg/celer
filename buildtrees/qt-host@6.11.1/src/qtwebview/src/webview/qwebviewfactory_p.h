// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QWEBVIEWFACTORY_H
#define QWEBVIEWFACTORY_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qwebview_p.h"
#include "qwebviewloadinginfo.h"

QT_BEGIN_NAMESPACE

class QWebViewPlugin;

namespace QWebViewFactory {
enum class Hint {
    NoHint = 0,
    QuickInitialization = 0x00000001,
};
struct LoadingInfo // wrapper to use private constructor
{
    static QWebViewLoadingInfo create(const QUrl &url, QWebViewLoadingInfo::LoadStatus status,
                                      const QString &errorString)
    {
        return QWebViewLoadingInfo(url, status, errorString);
    }
};
QWebViewPlugin *getPlugin();
QWebViewPrivate *createWebView(QWebView *view, Hint hint = Hint::NoHint);
bool requiresExtraInitializationSteps();
Q_WEBVIEW_EXPORT bool loadedPluginHasKey(const QString key);
}; // namespace QWebViewFactory

QT_END_NAMESPACE

#endif // QWEBVIEWFACTORY_H
