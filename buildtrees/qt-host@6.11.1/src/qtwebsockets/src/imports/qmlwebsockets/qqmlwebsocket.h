// Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQMLWEBSOCKET_H
#define QQMLWEBSOCKET_H

#include <QObject>
#include <QQmlParserStatus>
#include <QtQml>
#include <QScopedPointer>
#include <QtWebSockets/QWebSocket>

QT_BEGIN_NAMESPACE

class QQmlWebSocket : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_DISABLE_COPY(QQmlWebSocket)
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(QStringList requestedSubprotocols READ requestedSubprotocols
               WRITE setRequestedSubprotocols NOTIFY requestedSubprotocolsChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activeChanged)
    Q_PROPERTY(QString negotiatedSubprotocol READ negotiatedSubprotocol NOTIFY
                       negotiatedSubprotocolChanged)

    QML_NAMED_ELEMENT(WebSocket)
    QML_ADDED_IN_VERSION(1, 0)

public:
    explicit QQmlWebSocket(QObject *parent = 0);
    explicit QQmlWebSocket(QWebSocket *socket, QObject *parent = 0);
    ~QQmlWebSocket() override;

    enum Status
    {
        Connecting  = 0,
        Open        = 1,
        Closing     = 2,
        Closed      = 3,
        Error       = 4
    };
    Q_ENUM(Status)

    QUrl url() const;
    void setUrl(const QUrl &url);
    QStringList requestedSubprotocols() const;
    void setRequestedSubprotocols(const QStringList &subprotocols);

    QString negotiatedSubprotocol() const;
    Status status() const;
    QString errorString() const;

    void setActive(bool active);
    bool isActive() const;

    Q_INVOKABLE qint64 sendTextMessage(const QString &message);
    Q_REVISION(1) Q_INVOKABLE qint64 sendBinaryMessage(const QByteArray &message);
    Q_REVISION(6, 10) Q_INVOKABLE void ping();
    Q_REVISION(6, 10) Q_INVOKABLE void ping(const QByteArray &payload);

Q_SIGNALS:
    void textMessageReceived(QString message);
    Q_REVISION(1) void binaryMessageReceived(QByteArray message);
    void statusChanged(QQmlWebSocket::Status status);
    void activeChanged(bool isActive);
    void errorStringChanged(QString errorString);
    void urlChanged();
    void requestedSubprotocolsChanged();
    void negotiatedSubprotocolChanged();
    Q_REVISION(6, 10) void pong(quint64 elapsedTime, const QByteArray &payload);

public:
    void classBegin() override;
    void componentComplete() override;

private Q_SLOTS:
    void onError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);

private:
    QScopedPointer<QWebSocket> m_webSocket;
    QString m_negotiatedProtocol;
    Status m_status;
    QUrl m_url;
    QStringList m_requestedProtocols;
    bool m_isActive;
    bool m_componentCompleted;
    QString m_errorString;

    // takes ownership of the socket
    void setSocket(QWebSocket *socket);
    void setStatus(Status status);
    void open();
    void close();
    void setErrorString(QString errorString = QString());
};

QT_END_NAMESPACE

#endif // QQMLWEBSOCKET_H
