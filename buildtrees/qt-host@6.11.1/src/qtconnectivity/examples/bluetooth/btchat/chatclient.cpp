// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "chatclient.h"

#include <QMetaEnum>
#include <QMetaObject>

#include <QBluetoothServiceInfo>

using namespace  Qt::StringLiterals;

ChatClient::ChatClient(QObject *parent)
    :   QObject(parent)
{
}

ChatClient::~ChatClient()
{
    stopClient();
}

//! [startClient]
void ChatClient::startClient(const QBluetoothServiceInfo &remoteService)
{
    if (socket)
        return;

    // Connect to service
    socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    qDebug() << "Create socket";
    socket->connectToService(remoteService);
    qDebug() << "ConnectToService done";

    connect(socket, &QBluetoothSocket::readyRead, this, &ChatClient::readSocket);
    connect(socket, &QBluetoothSocket::connected, this, QOverload<>::of(&ChatClient::connected));
    connect(socket, &QBluetoothSocket::disconnected, this, &ChatClient::disconnected);
    connect(socket, &QBluetoothSocket::errorOccurred, this, &ChatClient::onSocketErrorOccurred);
}
//! [startClient]

//! [stopClient]
void ChatClient::stopClient()
{
    delete socket;
    socket = nullptr;
}
//! [stopClient]

//! [readSocket]
void ChatClient::readSocket()
{
    if (!socket)
        return;

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();
        emit messageReceived(socket->peerName(),
                             QString::fromUtf8(line.constData(), line.length()));
    }
}
//! [readSocket]

//! [sendMessage]
void ChatClient::sendMessage(const QString &message)
{
    QByteArray text = message.toUtf8() + '\n';
    socket->write(text);
}
//! [sendMessage]

void ChatClient::onSocketErrorOccurred(QBluetoothSocket::SocketError error)
{
    if (error == QBluetoothSocket::SocketError::NoSocketError)
        return;

    QMetaEnum metaEnum = QMetaEnum::fromType<QBluetoothSocket::SocketError>();
    QString errorString = socket->peerName() + ' '_L1
            + metaEnum.valueToKey(static_cast<int>(error)) + " occurred"_L1;

    emit socketErrorOccurred(errorString);
}

//! [connected]
void ChatClient::connected()
{
    emit connected(socket->peerName());
}
//! [connected]
