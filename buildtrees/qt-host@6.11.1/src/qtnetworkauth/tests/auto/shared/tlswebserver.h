// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef TLSWEBSERVER_H
#define TLSWEBSERVER_H

#include "webserver.h"

#ifndef QT_NO_SSL

#include <QtNetwork/qsslserver.h>

#include <QtCore/qmap.h>
#include <QtCore/qset.h>

QT_BEGIN_NAMESPACE
class QTcpSocket;
class QSslConfiguration;
class QUrl;
QT_END_NAMESPACE

class TlsWebServer : public QSslServer
{
public:
    using HttpRequest = WebServer::HttpRequest;
    using Handler = WebServer::Handler;

    TlsWebServer(Handler handler, const QSslConfiguration &config, QObject *parent = nullptr);
    QUrl url(const QString &path);
    void setExpectedSslErrors(const QSet<QSslError::SslError> &errors);

private:
    Handler handler;
    QMap<QTcpSocket *, HttpRequest> clients;
    QSet<QSslError::SslError> expectedSslErrors;
};

#endif // !QT_NO_SSL

#endif // TLSWEBSERVER_H
