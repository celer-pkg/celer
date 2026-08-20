// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "webserver.h"

#include <QtNetwork/qtcpsocket.h>

#include <QtCore/qcoreapplication.h>

#include <cctype>
#include <utility>

WebServer::WebServer(Handler h, QObject *parent) :
    QTcpServer(parent),
    handler(h)
{
    connect(this, &QTcpServer::newConnection, this, [this]() {
        auto socket = nextPendingConnection();
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
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
                if (request->headers.contains("host")) {
                    const auto parts = request->headers["host"].split(':');
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

    const auto ok = listen(QHostAddress::LocalHost);
    Q_ASSERT(ok);
}

QUrl WebServer::url(const QString &path)
{
    const QString format("http://127.0.0.1:%1%2");
    return QUrl(format.arg(serverPort()).arg(path.startsWith('/') ? path : "/" + path));
}

bool WebServer::HttpRequest::readMethod(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        const auto c = socket->read(1).at(0);
        if (std::isspace(c))
            finished = true;
        else if (std::isupper(c) && fragment.size() < 8)
            fragment += c;
        else
            return false;
    }
    if (finished) {
        if (fragment == "HEAD")
            method = Method::Head;
        else if (fragment == "GET")
            method = Method::Get;
        else if (fragment == "PUT")
            method = Method::Put;
        else if (fragment == "POST")
            method = Method::Post;
        else if (fragment == "DELETE")
            method = Method::Delete;
        else
            qWarning("Invalid operation %s", fragment.data());

        state = State::ReadingUrl;
        fragment.clear();

        return method != Method::Unknown;
    }
    return true;
}

bool WebServer::HttpRequest::readUrl(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        const auto c = socket->read(1).at(0);
        if (std::isspace(c))
            finished = true;
        else
            fragment += c;
    }
    if (finished) {
        if (!fragment.startsWith("/")) {
            qWarning("Invalid URL path %s", fragment.constData());
            return false;
        }
        url.setUrl(QStringLiteral("http://127.0.0.1:") + QString::number(port) +
                   QString::fromUtf8(fragment));
        state = State::ReadingStatus;
        if (!url.isValid()) {
            qWarning("Invalid URL %s", fragment.constData());
            return false;
        }
        fragment.clear();
    }
    return true;
}

bool WebServer::HttpRequest::readStatus(QTcpSocket *socket)
{
    bool finished = false;
    while (socket->bytesAvailable() && !finished) {
        fragment += socket->read(1);
        if (fragment.endsWith("\r\n")) {
            finished = true;
            fragment.resize(fragment.size() - 2);
        }
    }
    if (finished) {
        if (!std::isdigit(fragment.at(fragment.size() - 3)) ||
                fragment.at(fragment.size() - 2) != '.' ||
                !std::isdigit(fragment.at(fragment.size() - 1))) {
            qWarning("Invalid version");
            return false;
        }
        version = std::make_pair(fragment.at(fragment.size() - 3) - '0',
                            fragment.at(fragment.size() - 1) - '0');
        state = State::ReadingHeader;
        fragment.clear();
    }
    return true;
}

bool WebServer::HttpRequest::readHeaders(QTcpSocket *socket)
{
    while (socket->bytesAvailable()) {
        fragment += socket->read(1);
        if (fragment.endsWith("\r\n")) {
            if (fragment == "\r\n") {
                state = State::ReadingBody;
                fragment.clear();
                return true;
            } else {
                fragment.chop(2);
                const int index = fragment.indexOf(':');
                if (index == -1)
                    return false;

                const QByteArray key = fragment.mid(0, index).trimmed();
                const QByteArray value = fragment.mid(index + 1).trimmed();
                headers.insert(key.toLower(), value);
                fragment.clear();
            }
        }
    }
    return true;
}

bool WebServer::HttpRequest::readBody(QTcpSocket *socket)
{
    if (headers.contains("content-length")) {
        bool conversionResult;
        bytesLeft = headers["content-length"].toInt(&conversionResult);
        if (Q_UNLIKELY(!conversionResult))
            return false;
        fragment.resize(bytesLeft);
    }
    while (bytesLeft) {
        int got = socket->read(&fragment.data()[fragment.size() - bytesLeft], bytesLeft);
        if (got < 0)
            return false; // error
        bytesLeft -= got;
        if (bytesLeft)
            qApp->processEvents();
    }
    fragment.swap(body);
    state = State::AllDone;
    return true;
}
