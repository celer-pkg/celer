// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 Intel Corporation.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QABSTRACTSOCKETENGINERECEIVER_P_H
#define QABSTRACTSOCKETENGINERECEIVER_P_H

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

#include <QtNetwork/private/qtnetworkglobal_p.h>



QT_BEGIN_NAMESPACE

class QAuthenticator;
class QNetworkProxy;

class Q_NETWORK_EXPORT QAbstractSocketEngineReceiver {
public:
    virtual ~QAbstractSocketEngineReceiver() = 0; // impl. in qabstractsocketengine.cpp
    virtual void readNotification()= 0;
    virtual void writeNotification()= 0;
    virtual void closeNotification()= 0;
    virtual void exceptionNotification()= 0;
    virtual void connectionNotification()= 0;
#ifndef QT_NO_NETWORKPROXY
    virtual void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *authenticator)= 0;
#endif
};

QT_END_NAMESPACE

#endif // QABSTRACTSOCKETENGINERECEIVER_P_H
