// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qmodbusrtuserialserver.h"
#include "qmodbusrtuserialserver_p.h"

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

/*!
    \class QModbusRtuSerialServer
    \inmodule QtSerialBus
    \since 6.2

    \brief The QModbusRtuSerialServer class represents a Modbus server
    that uses a serial port for its communication with the Modbus client.

    Communication via Modbus requires the interaction between a single Modbus
    client instance and multiple Modbus server. This class provides the Modbus
    server implementation via a serial port.

    Since multiple Modbus server instances can interact with a Modbus client
    at the same time (using a serial bus), servers are identified by their
    \l serverAddress().
*/

/*!
    Constructs a QModbusRtuSerialServer with the specified \a parent. The
    \l serverAddress preset is \c 1.
*/
QModbusRtuSerialServer::QModbusRtuSerialServer(QObject *parent)
    : QModbusServer(*new QModbusRtuSerialServerPrivate, parent)
{
    Q_D(QModbusRtuSerialServer);
    d->setupSerialPort();
}

/*!
    Destroys the QModbusRtuSerialServer instance.
*/
QModbusRtuSerialServer::~QModbusRtuSerialServer()
{
    close();
}

/*!
    \internal
*/
QModbusRtuSerialServer::QModbusRtuSerialServer(QModbusRtuSerialServerPrivate &dd, QObject *parent)
    : QModbusServer(dd, parent)
{
    Q_D(QModbusRtuSerialServer);
    d->setupSerialPort();
}

/*!
    \reimp
*/
bool QModbusRtuSerialServer::processesBroadcast() const
{
    return d_func()->m_processesBroadcast;
}

/*!
    \since 6.2

    Returns the amount of microseconds for the silent interval between two
    consecutive Modbus messages.

    \sa setInterFrameDelay()
*/
int QModbusRtuSerialServer::interFrameDelay() const
{
    Q_D(const QModbusRtuSerialServer);
    return d->m_interFrameDelayMilliseconds * 1000;
}

/*!
    \since 6.2

    Sets the amount of \a microseconds for the silent interval between two
    consecutive Modbus messages. By default, the class implementation will use
    a pre-calculated value according to the Modbus specification. A active or
    running connection is not affected by such delay changes.

    \note If \a microseconds is set to -1 or \a microseconds is less than the
    pre-calculated delay then this pre-calculated value is used as frame delay.
*/
void QModbusRtuSerialServer::setInterFrameDelay(int microseconds)
{
    Q_D(QModbusRtuSerialServer);
    d->m_interFrameDelayMilliseconds = qCeil(qreal(microseconds) / 1000.);
    d->calculateInterFrameDelay();
}

/*!
    \reimp

     \note When calling this function, existing buffered data is removed from
     the serial port.
*/
bool QModbusRtuSerialServer::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    Q_D(QModbusRtuSerialServer);
    d->setupEnvironment(); // to be done before open
    if (d->m_serialPort->open(QIODevice::ReadWrite)) {
        setState(QModbusDevice::ConnectedState);
        d->m_serialPort->clear(); // only possible after open
    } else {
        setError(d->m_serialPort->errorString(), QModbusDevice::ConnectionError);
    }
    return (state() == QModbusDevice::ConnectedState);
}

/*!
    \reimp
*/
void QModbusRtuSerialServer::close()
{
    if (state() == QModbusDevice::UnconnectedState)
        return;

    Q_D(QModbusRtuSerialServer);
    if (d->m_serialPort->isOpen())
        d->m_serialPort->close();

    setState(QModbusDevice::UnconnectedState);
}

/*!
    \reimp

    Processes the Modbus client request specified by \a request and returns a
    Modbus response.

    The Modbus function \l QModbusRequest::EncapsulatedInterfaceTransport with
    MEI Type 13 (0x0D) CANopen General Reference is filtered out because it is
    usually Modbus TCP or Modbus serial ASCII only.

    A request to the RTU serial server will be answered with a Modbus exception
    response with the exception code QModbusExceptionResponse::IllegalFunction.
*/
QModbusResponse QModbusRtuSerialServer::processRequest(const QModbusPdu &request)
{
    if (request.functionCode() == QModbusRequest::EncapsulatedInterfaceTransport) {
        quint8 meiType = 0;
        request.decodeData(&meiType);
        if (meiType == EncapsulatedInterfaceTransport::CanOpenGeneralReference) {
            return QModbusExceptionResponse(request.functionCode(),
                QModbusExceptionResponse::IllegalFunction);
        }
    }
    return QModbusServer::processRequest(request);
}

QT_END_NAMESPACE
