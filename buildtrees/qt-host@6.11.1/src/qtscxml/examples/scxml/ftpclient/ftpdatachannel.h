// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef FTPDATACHANNEL_H
#define FTPDATACHANNEL_H

#include <QtCore/qobject.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>

#include <memory>

class FtpDataChannel : public QObject
{
    Q_OBJECT
public:
    explicit FtpDataChannel(QObject *parent = nullptr);

    // Listen on a local address.
    void listen(const QHostAddress &address = QHostAddress::Any);

    // Send data over the socket.
    void sendData(const QByteArray &data);

    // Close the connection.
    void close();

    // Retrieve the port specification to be announced on the control channel.
    // Something like "a,b,c,d,xxx,yyy" where
    // - a.b.c.d would be the IP address in decimal/dot notation and
    // - xxx,yyy are the upper and lower 8 bits of the TCP port in decimal
    // (This will only work if the local address we're listening on
    // is actually meaningful)
    QString portspec() const;

signals:

    // The FTP server has sent some data.
    void dataReceived(const QByteArray &data);

private:
    QTcpServer m_server;
    std::unique_ptr<QTcpSocket> m_socket;
};

#endif // FTPDATACHANNEL_H
