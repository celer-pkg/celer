// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qmodbusrtuserialclient.h"
#include "qmodbusrtuserialclient_p.h"

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS)
Q_DECLARE_LOGGING_CATEGORY(QT_MODBUS_LOW)

/*!
    \class QModbusRtuSerialClient
    \inmodule QtSerialBus
    \since 6.2

    \brief The QModbusRtuSerialClient class represents a Modbus client
    that uses a serial bus for its communication with the Modbus server.

    Communication via Modbus requires the interaction between a single
    Modbus client instance and multiple Modbus servers. This class
    provides the client implementation via a serial port.
*/

/*!
    Constructs a serial Modbus client with the specified \a parent.
*/
QModbusRtuSerialClient::QModbusRtuSerialClient(QObject *parent)
    : QModbusClient(*new QModbusRtuSerialClientPrivate, parent)
{
    Q_D(QModbusRtuSerialClient);
    d->setupSerialPort();
}

/*!
    \internal
*/
QModbusRtuSerialClient::~QModbusRtuSerialClient()
{
    close();
}

/*!
    Returns the amount of microseconds for the silent interval between two
    consecutive Modbus messages.

    \sa setInterFrameDelay()
*/
int QModbusRtuSerialClient::interFrameDelay() const
{
    Q_D(const QModbusRtuSerialClient);
    return d->m_interFrameDelayMilliseconds * 1000;
}

/*!
    Sets the amount of \a microseconds for the silent interval between two
    consecutive Modbus messages. By default, the class implementation will use
    a pre-calculated value according to the Modbus specification. A active or
    running connection is not affected by such delay changes.

    \note If \a microseconds is set to -1 or \a microseconds is less than the
    pre-calculated delay then this pre-calculated value is used as frame delay.
*/
void QModbusRtuSerialClient::setInterFrameDelay(int microseconds)
{
    Q_D(QModbusRtuSerialClient);
    d->m_interFrameDelayMilliseconds = qCeil(qreal(microseconds) / 1000.);
    d->calculateInterFrameDelay();
}

/*!
    \since 5.13

    Returns the amount of milliseconds for the silent interval between a Modbus
    broadcast and a consecutive Modbus messages. The default value is set to
    \c 100 milliseconds.
*/
int QModbusRtuSerialClient::turnaroundDelay() const
{
    Q_D(const QModbusRtuSerialClient);
    return d->m_turnaroundDelay;
}

/*!
    \since 5.13

    Sets the amount of milliseconds for the silent interval between a Modbus
    broadcast and a consecutive Modbus messages to \a turnaroundDelay.
    Typically the turnaround delay is in the range of \c 100 to \c 200
    milliseconds.
*/
void QModbusRtuSerialClient::setTurnaroundDelay(int turnaroundDelay)
{
    Q_D(QModbusRtuSerialClient);
    d->m_turnaroundDelay = turnaroundDelay;
}

/*!
    \internal
*/
QModbusRtuSerialClient::QModbusRtuSerialClient(QModbusRtuSerialClientPrivate &dd, QObject *parent)
    : QModbusClient(dd, parent)
{
    Q_D(QModbusRtuSerialClient);
    d->setupSerialPort();
}

/*!
     \reimp

     \note When calling this function, existing buffered data is removed from
     the serial port.
*/
bool QModbusRtuSerialClient::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    Q_D(QModbusRtuSerialClient);
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
void QModbusRtuSerialClient::close()
{
    if (state() == QModbusDevice::UnconnectedState)
        return;

    setState(QModbusDevice::ClosingState);

    Q_D(QModbusRtuSerialClient);

    if (d->m_serialPort->isOpen())
        d->m_serialPort->close();

    int numberOfAborts = 0;
    while (!d->m_queue.isEmpty()) {
        // Finish each open reply and forget them
        QModbusRtuSerialClientPrivate::QueueElement elem = d->m_queue.dequeue();
        if (!elem.reply.isNull()) {
            elem.reply->setError(QModbusDevice::ReplyAbortedError,
                                 QModbusClient::tr("Reply aborted due to connection closure."));
            numberOfAborts++;
        }
    }

    if (numberOfAborts > 0)
        qCDebug(QT_MODBUS_LOW) << "(RTU client) Aborted replies:" << numberOfAborts;

    setState(QModbusDevice::UnconnectedState);
}

QT_END_NAMESPACE

#include "moc_qmodbusrtuserialclient.cpp"
