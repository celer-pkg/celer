// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of the Network Access API.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QOAUTHURISCHEMEREPLYHANDLER_P_H
#define QOAUTHURISCHEMEREPLYHANDLER_P_H

#include <QtNetworkAuth/qoauthglobal.h>
#include <QtNetworkAuth/qoauthurischemereplyhandler.h>

#include <private/qoauthoobreplyhandler_p.h>

#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QOAuthUriSchemeReplyHandlerPrivate : public QOAuthOobReplyHandlerPrivate
{
    Q_DECLARE_PUBLIC(QOAuthUriSchemeReplyHandler)

public:
    bool hasValidRedirectUrl() const;
    bool _q_handleRedirectUrl(const QUrl &url);

    QUrl redirectUrl;
    bool forwardUnhandledUrls = true;
    bool listening = false;
};

QT_END_NAMESPACE

#endif // QOAUTHURISCHEMEREPLYHANDLER_P_H
