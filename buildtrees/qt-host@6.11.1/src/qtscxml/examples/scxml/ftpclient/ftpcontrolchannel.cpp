// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "ftpcontrolchannel.h"

#include <QtCore/qcoreapplication.h>

FtpControlChannel::FtpControlChannel(QObject *parent) : QObject(parent)
{
    connect(&m_socket, &QIODevice::readyRead,
            this, &FtpControlChannel::onReadyRead);
    connect(&m_socket, &QAbstractSocket::disconnected,
            this, &FtpControlChannel::closed);
    connect(&m_socket, &QAbstractSocket::connected, this, [this]() {
        emit opened(m_socket.localAddress(), m_socket.localPort());
    });
    connect(&m_socket, &QAbstractSocket::errorOccurred,
            this, &FtpControlChannel::error);
}

void FtpControlChannel::connectToServer(const QString &server)
{
    m_socket.connectToHost(server, 21);
}

void FtpControlChannel::command(const QByteArray &command,
                                const QByteArray &params)
{
    QByteArray sendData = command;
    if (!params.isEmpty())
        sendData += " " + params;
    m_socket.write(sendData + "\r\n");
}

void FtpControlChannel::onReadyRead()
{
    m_buffer.append(m_socket.readAll());
    int rn = -1;
    while ((rn = m_buffer.indexOf("\r\n")) != -1) {
        QByteArray received = m_buffer.mid(0, rn);
        m_buffer = m_buffer.mid(rn + 2);
        int space = received.indexOf(' ');
        if (space != -1) {
            int code = received.mid(0, space).toInt();
            if (code == 0) {
                qDebug() << "Info received: " << received.mid(space + 1);
                emit info(received.mid(space + 1));
            } else {
                qDebug() << "Reply received: " << received.mid(space + 1);
                emit reply(code, received.mid(space + 1));
            }
        } else {
            emit invalidReply(received);
        }
    }
}

void FtpControlChannel::error(QAbstractSocket::SocketError error)
{
    qWarning() << "Socket error:" << error;
    QCoreApplication::exit();
}
