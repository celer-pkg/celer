// Copyright (C) 2017-2016 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QQNXNATIVEIO_H
#define QQNXNATIVEIO_H

#include <QtNetwork/qabstractsocket.h>
#include <QtRemoteObjects/qtremoteobjectglobal.h>

QT_BEGIN_NAMESPACE

/*
 * The implementation of the Source and Replica
 * side QIODevice look like they will be fairly
 * different, including different APIs.  So
 * creating a 2nd derived type for the source.
 *
 * TODO: revisit if these can be combined into a
 * single type.
 *
 * With two types, QQnxNativeIo will need to get
 * Source or Replica added.  Not sure what intuitive
 * names are yet.  So for now, QQnxNativeIo is the
 * Replica side, QIOQnxSourcePrivate is the Source
 * side.  Revisit the name as this matures.
 *
*/
class QQnxNativeIoPrivate;
class QIOQnxSourcePrivate;

class Q_REMOTEOBJECTS_EXPORT QQnxNativeIo : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQnxNativeIo)

public:
    explicit QQnxNativeIo(QObject *parent = nullptr);
    ~QQnxNativeIo() override;

    bool connectToServer(OpenMode openMode = ReadWrite);
    bool connectToServer(const QString &name, OpenMode openMode = ReadWrite);
    void disconnectFromServer();

    void setServerName(const QString &name);
    QString serverName() const;

    void abort();
    bool isSequential() const override;
    qint64 bytesAvailable() const override;
    qint64 bytesToWrite() const override;
    bool open(OpenMode openMode = ReadWrite) override;
    void close() override;
    QAbstractSocket::SocketError error() const;
    bool flush();
    bool isValid() const;

    QAbstractSocket::SocketState state() const;
    bool waitForBytesWritten(int msecs = 30000) override;
    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000) override;

Q_SIGNALS:
    void connected();
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);

protected:
    qint64 readData(char*, qint64) override;
    qint64 writeData(const char*, qint64) override;

private:
    Q_DISABLE_COPY(QQnxNativeIo)
};

class Q_REMOTEOBJECTS_EXPORT QIOQnxSource : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QIOQnxSource)

public:
    explicit QIOQnxSource(int rcvid, QObject *parent = nullptr);
    ~QIOQnxSource() override;

    bool isSequential() const override;
    qint64 bytesAvailable() const override;
    qint64 bytesToWrite() const override;
    void close() override;
    QAbstractSocket::SocketError error() const;
    bool isValid() const;

    QAbstractSocket::SocketState state() const;
    bool waitForBytesWritten(int msecs = 30000) override;
    bool waitForConnected(int msecs = 30000);
    bool waitForDisconnected(int msecs = 30000);
    bool waitForReadyRead(int msecs = 30000) override;

Q_SIGNALS:
    void disconnected();
    void error(QAbstractSocket::SocketError socketError);
    void stateChanged(QAbstractSocket::SocketState socketState);

protected:
    qint64 readData(char*, qint64) override;
    qint64 writeData(const char*, qint64) override;
    bool open(OpenMode openMode) override;

private Q_SLOTS:
    void onDisconnected();

private:
    Q_DISABLE_COPY(QIOQnxSource)
    friend class QQnxNativeServerPrivate;
    friend class QnxServerIo;
};

QT_END_NAMESPACE

#endif // QQNXNATIVEIO_H
