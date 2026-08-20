// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef MODBUSTCPCLIENT_P_H
#define MODBUSTCPCLIENT_P_H

#include "modbustcpclient.h"

#include <QDebug>
#include <QModbusReply>

#include <private/qmodbustcpclient_p.h>

QT_USE_NAMESPACE

class ModbusTcpClientPrivate : private QModbusTcpClientPrivate
{
    Q_DECLARE_PUBLIC(ModbusTcpClient)

public:
    QModbusReply *enqueueRequest(const QModbusRequest &request, int, const QModbusDataUnit &unit,
                                 QModbusReply::ReplyType type) override
    {
        auto writeToSocket = [this](const QModbusRequest &request) {
            QByteArray buffer;
            QDataStream output(&buffer, QIODevice::WriteOnly);
            output << m_tId << m_pId << m_length << m_uId << request;

            qint64 writtenBytes = m_socket->write(buffer);
            if (writtenBytes == -1 || writtenBytes < buffer.size()) {
                Q_Q(ModbusTcpClient);
                qDebug() << "Cannot write request to socket.";
                q->setError(QModbusTcpClient::tr("Could not write request to socket."),
                            QModbusDevice::WriteError);
                return false;
            }
            qDebug() << "Sent TCP ADU:" << buffer.toHex();
            qDebug() << "Sent TCP PDU:" << request << "with tId:" << Qt::hex << m_tId;
            return true;
        };

        if (!writeToSocket(request))
            return nullptr;

        Q_Q(ModbusTcpClient);
        auto reply = new QModbusReply(type, m_uId, q);
        const auto element = QueueElement{reply, request, unit, m_numberOfRetries,
            m_responseTimeoutDuration};
        m_transactionStore.insert(m_tId, element);

        q->connect(q, &QModbusClient::timeoutChanged,
                   element.timer.data(), QOverload<int>::of(&QTimer::setInterval));
        QObject::connect(element.timer.data(), &QTimer::timeout, q, [this, writeToSocket]() {
            if (!m_transactionStore.contains(m_tId))
                return;

            QueueElement elem = m_transactionStore.take(m_tId);
            if (elem.reply.isNull())
                return;

            if (elem.numberOfRetries > 0) {
                elem.numberOfRetries--;
                if (!writeToSocket(elem.requestPdu))
                    return;
                m_transactionStore.insert(m_tId, elem);
                elem.timer->start();
                qDebug() << "Resend request with tId:" << Qt::hex << m_tId;
            } else {
                qDebug() << "Timeout of request with tId:" << Qt::hex << m_tId;
                elem.reply->setError(QModbusDevice::TimeoutError,
                    QModbusClient::tr("Request timeout."));
            }
        });
        element.timer->start();
        return reply;
    }

    quint16 m_tId = 0;
    quint16 m_pId = 0;
    quint16 m_length = 0;
    quint8 m_uId = 0;
};

#endif // MODBUSTCPCLIENT_P_H
