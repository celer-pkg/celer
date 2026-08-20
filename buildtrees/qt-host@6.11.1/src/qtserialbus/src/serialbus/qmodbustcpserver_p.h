// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#ifndef QMODBUSTCPSERVER_P_H
#define QMODBUSTCPSERVER_P_H

#include <QtCore/qdatastream.h>
#include <QtCore/qdebug.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qobject.h>
#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtSerialBus/qmodbustcpserver.h>

#include <private/qmodbusserver_p.h>

#include <memory>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS)
Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS_LOW)

class QModbusTcpServerPrivate : public QModbusServerPrivate
{
    Q_DECLARE_PUBLIC(QModbusTcpServer)

public:
    /*
        This function is a workaround since 2nd level lambda below cannot
        call protected QModbusTcpServer::processRequest(..) function on VS2013.
    */
    QModbusResponse forwardProcessRequest(const QModbusRequest &r)
    {
        Q_Q(QModbusTcpServer);
        if (q->value(QModbusServer::DeviceBusy).value<quint16>() == 0xffff) {
            // If the device is busy, send an exception response without processing.
            incrementCounter(QModbusServerPrivate::Counter::ServerBusy);
            return QModbusExceptionResponse(r.functionCode(),
                QModbusExceptionResponse::ServerDeviceBusy);
        }
        return q->processRequest(r);
    }

    /*
        This function is a workaround since 2nd level lambda below cannot
        call protected QModbusDevice::setError(..) function on VS2013.
    */
    void forwardError(const QString &errorText, QModbusDevice::Error error)
    {
        Q_Q(QModbusTcpServer);
        q->setError(errorText, error);
    }

    /*
        This function is a workaround since 2nd level lambda below
        cannot call QModbusServer::serverAddress(..) function on VS2013.
    */
    bool matchingServerAddress(quint8 unitId) const
    {
        Q_Q(const QModbusTcpServer);
        if (q->serverAddress() == unitId)
            return true;

        // No, not our address! Ignore!
        qCDebug(QT_MODBUS) << "(TCP server) Wrong server unit identifier address, expected"
            << q->serverAddress() << "got" << unitId;
        return false;
    }

    void setupTcpServer()
    {
        m_tcpServer = new QTcpServer(q_func());
        QObject::connect(m_tcpServer, &QTcpServer::newConnection, q_func(), [this]() {
            Q_Q(QModbusTcpServer);
            auto *socket = m_tcpServer->nextPendingConnection();
            if (!socket)
                return;

            qCDebug(QT_MODBUS) << "(TCP server) Incoming socket from" << socket->peerAddress()
                               << socket->peerName() << socket->peerPort();

            if (m_observer && !m_observer->acceptNewConnection(socket)) {
                qCDebug(QT_MODBUS) << "(TCP server) Connection rejected by observer";
                socket->close();
                socket->deleteLater();
                return;
            }

            auto buffer = new QByteArray();

            QObject::connect(socket, &QObject::destroyed, socket, [buffer]() {
                // cleanup buffer
                delete buffer;
            });
            QObject::connect(socket, &QTcpSocket::disconnected, q, [socket, this]() {
                Q_Q(QModbusTcpServer);
                emit q->modbusClientDisconnected(socket);
                socket->deleteLater();
            });
            QObject::connect(socket, &QTcpSocket::readyRead, q, [buffer, socket, this]() {
                if (!socket)
                    return;

                buffer->append(socket->readAll());
                while (!buffer->isEmpty()) {
                    qCDebug(QT_MODBUS_LOW).noquote() << "(TCP server) Read buffer: 0x"
                        + buffer->toHex();

                    if (buffer->size() < mbpaHeaderSize) {
                        qCDebug(QT_MODBUS) << "(TCP server) MBPA header too short. Waiting for more data.";
                        return;
                    }

                    quint8 unitId;
                    quint16 transactionId, bytesPdu, protocolId;
                    QDataStream input(*buffer);
                    input >> transactionId >> protocolId >> bytesPdu >> unitId;

                    qCDebug(QT_MODBUS_LOW) << "(TCP server) Request MBPA:" << "Transaction Id:"
                        << Qt::hex << transactionId << "Protocol Id:" << protocolId << "PDU bytes:"
                        << bytesPdu << "Unit Id:" << unitId;

                    // The length field is the byte count of the following fields, including the Unit
                    // Identifier and the PDU, so we remove on byte.
                    bytesPdu--;

                    const quint16 current = mbpaHeaderSize + bytesPdu;
                    if (buffer->size() < current) {
                        qCDebug(QT_MODBUS) << "(TCP server) PDU too short. Waiting for more data";
                        return;
                    }

                    QModbusRequest request;
                    input >> request;

                    buffer->remove(0, current);

                    if (!matchingServerAddress(unitId))
                        continue;

                    qCDebug(QT_MODBUS) << "(TCP server) Request PDU:" << request;
                    const QModbusResponse response = forwardProcessRequest(request);
                    qCDebug(QT_MODBUS) << "(TCP server) Response PDU:" << response;

                    QByteArray result;
                    QDataStream output(&result, QIODevice::WriteOnly);
                    // The length field is the byte count of the following fields, including the Unit
                    // Identifier and PDU fields, so we add one byte to the response size.
                    output << transactionId << protocolId << quint16(response.size() + 1)
                           << unitId << response;

                    if (!socket->isOpen()) {
                        qCDebug(QT_MODBUS) << "(TCP server) Requesting socket has closed.";
                        forwardError(QModbusTcpServer::tr("Requesting socket is closed"),
                                     QModbusDevice::WriteError);
                        return;
                    }

                    qint64 writtenBytes = socket->write(result);
                    if (writtenBytes == -1 || writtenBytes < result.size()) {
                        qCDebug(QT_MODBUS) << "(TCP server) Cannot write requested response to socket.";
                        forwardError(QModbusTcpServer::tr("Could not write response to client"),
                                     QModbusDevice::WriteError);
                    }
                }
            });
        });

        QObject::connect(m_tcpServer, &QTcpServer::acceptError, q_func(),
                         [this](QAbstractSocket::SocketError /*sError*/) {
            Q_Q(QModbusTcpServer);

            qCWarning(QT_MODBUS) << "(TCP server) Accept error";
            q->setError(m_tcpServer->errorString(), QModbusDevice::ConnectionError);
        });
    }

    QTcpServer *m_tcpServer { nullptr };

    std::unique_ptr<QModbusTcpConnectionObserver> m_observer;

    static const qint8 mbpaHeaderSize = 7;
    static const qint16 maxBytesModbusADU = 260;
};

QT_END_NAMESPACE

#endif // QMODBUSTCPSERVER_P_H

