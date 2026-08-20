// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FTPCONTROLCHANNEL_H
#define FTPCONTROLCHANNEL_H

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtCore/qobject.h>

class FtpControlChannel : public QObject
{
    Q_OBJECT
public:
    explicit FtpControlChannel(QObject *parent = nullptr);

    // Connect to an FTP server
    void connectToServer(const QString &server);

    // Send a command to the server
    void command(const QByteArray &command, const QByteArray &params);

public slots:
    void error(QAbstractSocket::SocketError);

signals:

    // Connection established. Local address and port are known.
    void opened(const QHostAddress &localAddress, int localPort);

    // Connection closed
    void closed();

    // Informational message
    void info(const QByteArray &info);

    // Reply to a previously sent command
    void reply(int code, const QByteArray &parameters);

    // Something is wrong
    void invalidReply(const QByteArray &reply);

private:
    void onReadyRead();

    QTcpSocket m_socket;
    QByteArray m_buffer;
};

#endif // FTPCONTROLCHANNEL_H
