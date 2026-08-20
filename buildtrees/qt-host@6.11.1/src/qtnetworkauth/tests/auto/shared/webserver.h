// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <QtNetwork/qtcpserver.h>

#include <QtCore/qmap.h>
#include <QtCore/qurl.h>

#include <functional>

QT_BEGIN_NAMESPACE
class QTcpSocket;
QT_END_NAMESPACE

class WebServer : public QTcpServer
{
public:
    class HttpRequest {
        friend class WebServer;
        friend class TlsWebServer;

        quint16 port = 0;
        enum class State {
            ReadingMethod,
            ReadingUrl,
            ReadingStatus,
            ReadingHeader,
            ReadingBody,
            AllDone
        } state = State::ReadingMethod;
        QByteArray fragment;
        int bytesLeft = 0;

        bool readMethod(QTcpSocket *socket);
        bool readUrl(QTcpSocket *socket);
        bool readStatus(QTcpSocket *socket);
        bool readHeaders(QTcpSocket *socket);
        bool readBody(QTcpSocket *socket);

    public:
        enum class Method {
            Unknown,
            Head,
            Get,
            Put,
            Post,
            Delete,
        } method = Method::Unknown;
        QUrl url;
        std::pair<quint8, quint8> version;
        QMap<QByteArray, QByteArray> headers;
        QByteArray body;
    };

    typedef std::function<void(const HttpRequest &request, QTcpSocket *socket)> Handler;

    WebServer(Handler handler, QObject *parent = nullptr);

    QUrl url(const QString &path);

private:
    Handler handler;

    QMap<QTcpSocket *, HttpRequest> clients;
};

#endif // WEBSERVER_H
