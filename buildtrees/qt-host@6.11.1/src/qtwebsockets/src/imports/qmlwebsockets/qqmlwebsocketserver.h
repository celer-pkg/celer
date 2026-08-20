// Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com, author Milian Wolff <milian.wolff@kdab.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQMLWEBSOCKETSERVER_H
#define QQMLWEBSOCKETSERVER_H

#include <QUrl>
#include <QQmlParserStatus>
#include <QtQml/qqmlregistration.h>
#include <QtWebSockets/QWebSocketServer>

QT_BEGIN_NAMESPACE

class QQmlWebSocket;
class QQmlWebSocketServer : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_DISABLE_COPY(QQmlWebSocketServer)
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)
    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QStringList supportedSubprotocols READ supportedSubprotocols
               WRITE setSupportedSubprotocols NOTIFY supportedSubprotocolsChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged)
    Q_PROPERTY(bool listen READ listen WRITE setListen NOTIFY listenChanged)
    Q_PROPERTY(bool accept READ accept WRITE setAccept NOTIFY acceptChanged)

    QML_NAMED_ELEMENT(WebSocketServer)
    QML_ADDED_IN_VERSION(1, 0)

public:
    explicit QQmlWebSocketServer(QObject *parent = nullptr);
    ~QQmlWebSocketServer() override;

    void classBegin() override;
    void componentComplete() override;

    QUrl url() const;

    QString host() const;
    void setHost(const QString &host);

    int port() const;
    void setPort(int port);

    QString name() const;
    void setName(const QString &name);

    QStringList supportedSubprotocols() const;
    void setSupportedSubprotocols(const QStringList &supportedSubprotocols);

    QString errorString() const;

    bool listen() const;
    void setListen(bool listen);

    bool accept() const;
    void setAccept(bool accept);

Q_SIGNALS:
    void clientConnected(QQmlWebSocket *webSocket);

    void errorStringChanged(const QString &errorString);
    void urlChanged(const QUrl &url);
    void portChanged(int port);
    void nameChanged(const QString &name);
    void supportedSubprotocolsChanged(const QStringList &supportedProtocols);
    void hostChanged(const QString &host);
    void listenChanged(bool listen);
    void acceptChanged(bool accept);

private:
    void init();
    void updateListening();
    void newConnection();
    void serverError();
    void closed();

    QScopedPointer<QWebSocketServer> m_server;
    QString m_host;
    QString m_name;
    QStringList m_supportedSubprotocols;
    int m_port;
    bool m_listen;
    bool m_accept;
    bool m_componentCompleted;

};

QT_END_NAMESPACE

#endif // QQMLWEBSOCKETSERVER_H
