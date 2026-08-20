// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "tlswebserver.h"

#ifndef QT_NO_SSL

#include <QtNetwork/qsslconfiguration.h>
#include <QtNetwork/qsslsocket.h>
#include <QtNetwork/qtcpserver.h>

#include <QtCore/qurl.h>

TlsWebServer::TlsWebServer(Handler h, const QSslConfiguration &config, QObject *parent) :
    QSslServer(parent),
    handler(h)
{
    connect(this, &QSslServer::pendingConnectionAvailable, this, [this]() {
        auto socket = nextPendingConnection();
        Q_ASSERT(socket);
        auto sslSocket = qobject_cast<QSslSocket *>(socket);
        Q_ASSERT(sslSocket);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
        connect(sslSocket, &QSslSocket::sslErrors, this, [sslSocket](const QList<QSslError> &errors) {
            qDebug() << errors;
            sslSocket->ignoreSslErrors();
        });
        connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
            if (!clients.contains(socket))
                clients[socket].port = serverPort();

            auto *request = &clients[socket];
            auto ok = true;

            while (socket->bytesAvailable()) {
                if (Q_LIKELY(request->state == HttpRequest::State::ReadingMethod))
                    if (Q_UNLIKELY(!(ok = request->readMethod(socket))))
                        qWarning("Invalid Method");

                if (Q_LIKELY(ok && request->state == HttpRequest::State::ReadingUrl))
                    if (Q_UNLIKELY(!(ok = request->readUrl(socket))))
                        qWarning("Invalid URL");

                if (Q_LIKELY(ok && request->state == HttpRequest::State::ReadingStatus))
                    if (Q_UNLIKELY(!(ok = request->readStatus(socket))))
                        qWarning("Invalid Status");

                if (Q_LIKELY(ok && request->state == HttpRequest::State::ReadingHeader))
                    if (Q_UNLIKELY(!(ok = request->readHeaders(socket))))
                        qWarning("Invalid Header");

                if (Q_LIKELY(ok && request->state == HttpRequest::State::ReadingBody))
                    if (Q_UNLIKELY(!(ok = request->readBody(socket))))
                        qWarning("Invalid Body");
            }
            if (Q_UNLIKELY(!ok)) {
                socket->disconnectFromHost();
                clients.remove(socket);
            } else if (Q_LIKELY(request->state == HttpRequest::State::AllDone)) {
                Q_ASSERT(handler);
                if (request->headers.contains("Host")) {
                    const auto parts = request->headers["Host"].split(':');
                    request->url.setHost(parts.at(0));
                    if (parts.size() == 2)
                        request->url.setPort(parts.at(1).toUInt());
                }
                handler(*request, socket);
                socket->disconnectFromHost();
                clients.remove(socket);
            }
        });
    });
    connect(this, &QSslServer::sslErrors, this, [this](QSslSocket *s, const QList<QSslError> &errors) {
        bool hasOnlyExpectedErrors = true;
        for (const auto &err : errors)
            hasOnlyExpectedErrors &= expectedSslErrors.contains(err.error());
        if (hasOnlyExpectedErrors)
            s->ignoreSslErrors();
        else
            qWarning() << "Got unexpected SSL errors" << errors;
    });

    setSslConfiguration(config);
    const bool ok = listen(QHostAddress::LocalHost);
    Q_ASSERT(ok);
}

QUrl TlsWebServer::url(const QString &path)
{
    using namespace Qt::StringLiterals;
    return QUrl(u"https://127.0.0.1:%1%2"_s.arg(serverPort()).arg(path.startsWith('/')
                                                                  ? path : "/" + path));
}

void TlsWebServer::setExpectedSslErrors(const QSet<QSslError::SslError> &errors)
{
    expectedSslErrors = errors;
}

#endif // !QT_NO_SSL
