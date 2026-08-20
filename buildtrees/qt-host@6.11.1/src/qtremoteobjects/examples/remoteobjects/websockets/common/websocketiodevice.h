// Copyright (C) 2019 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WEBSOCKETIODEVICE_H
#define WEBSOCKETIODEVICE_H

#include <QBuffer>
#include <QIODevice>
#include <QPointer>
#include <QWebSocket>

class WebSocketIoDevice : public QIODevice
{
    Q_OBJECT
public:
    WebSocketIoDevice(QWebSocket *webSocket, QObject *parent = nullptr);

signals:
    void disconnected();

    // QIODevice interface
public:
    qint64 bytesAvailable() const override;
    bool isSequential() const override;
    void close() override;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    QPointer<QWebSocket> m_socket;
    QByteArray m_buffer;
};

#endif // WEBSOCKETIODEVICE_H
