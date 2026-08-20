// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "websocketiodevice.h"

WebSocketIoDevice::WebSocketIoDevice(QWebSocket *webSocket, QObject *parent)
    : QIODevice(parent)
    , m_socket(webSocket)
{
    open(QIODevice::ReadWrite);
    connect(webSocket, &QWebSocket::disconnected, this, &WebSocketIoDevice::disconnected);
    connect(webSocket, &QWebSocket::binaryMessageReceived, this, [this](const QByteArray &message){
        m_buffer.append(message);
        emit readyRead();
    });
    connect(webSocket, &QWebSocket::bytesWritten, this, &WebSocketIoDevice::bytesWritten);
}

qint64 WebSocketIoDevice::bytesAvailable() const
{
    return QIODevice::bytesAvailable() + m_buffer.size();
}

bool WebSocketIoDevice::isSequential() const
{
    return true;
}

void WebSocketIoDevice::close()
{
    if (m_socket)
        m_socket->close();
}

qint64 WebSocketIoDevice::readData(char *data, qint64 maxlen)
{
    auto sz = std::min(maxlen, qint64(m_buffer.size()));
    if (sz <= 0)
        return sz;
    memcpy(data, m_buffer.constData(), size_t(sz));
    m_buffer.remove(0, sz);
    return sz;
}

qint64 WebSocketIoDevice::writeData(const char *data, qint64 len)
{
    if (m_socket)
        return m_socket->sendBinaryMessage(QByteArray{data, int(len)});
    return -1;
}
