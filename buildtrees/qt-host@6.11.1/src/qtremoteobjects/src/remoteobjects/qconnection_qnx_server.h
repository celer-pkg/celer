// Copyright (C) 2017-2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQNXNATIVESERVER_H
#define QQNXNATIVESERVER_H

#include <QtNetwork/qabstractsocket.h>
#include <QtRemoteObjects/qtremoteobjectglobal.h>

QT_BEGIN_NAMESPACE

class QQnxNativeServerPrivate;
class QQnxNativeIo;
class QIOQnxSource;

class Q_REMOTEOBJECTS_EXPORT QQnxNativeServer : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQnxNativeServer)

Q_SIGNALS:
    void newConnection();

public:
    explicit QQnxNativeServer(QObject *parent = nullptr);
    ~QQnxNativeServer();

    void close();
    bool hasPendingConnections() const;
    bool isListening() const;
    bool listen(const QString &name);
    QSharedPointer<QIOQnxSource> nextPendingConnection();
    QString serverName() const;
    bool waitForNewConnection(int msec = 0, bool *timedOut = nullptr);

private Q_SLOTS:
    void onSourceClosed();

private:
    Q_DISABLE_COPY(QQnxNativeServer)
    friend class QIOQnxSource;
};

QT_END_NAMESPACE

#endif // QQNXNATIVESERVER_H
