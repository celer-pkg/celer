// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qmodbustcpclient.h"
#include "qmodbustcpclient_p.h"

#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

/*!
    \class QModbusTcpClient
    \inmodule QtSerialBus
    \since 5.8

    \brief The QModbusTcpClient class is the interface class for Modbus TCP client device.

    QModbusTcpClient communicates with the Modbus backend providing users with a convenient API.
*/

/*!
    Constructs a QModbusTcpClient with the specified \a parent.
*/
QModbusTcpClient::QModbusTcpClient(QObject *parent)
    : QModbusClient(*new QModbusTcpClientPrivate, parent)
{
    Q_D(QModbusTcpClient);
    d->setupTcpSocket();
}

/*!
    Destroys the QModbusTcpClient instance.
*/
QModbusTcpClient::~QModbusTcpClient()
{
    close();
}

/*!
    \internal
*/
QModbusTcpClient::QModbusTcpClient(QModbusTcpClientPrivate &dd, QObject *parent)
    : QModbusClient(dd, parent)
{
    Q_D(QModbusTcpClient);
    d->setupTcpSocket();
}

/*!
     \reimp
*/
bool QModbusTcpClient::open()
{
    if (state() == QModbusDevice::ConnectedState)
        return true;

    Q_D(QModbusTcpClient);
    if (d->m_socket->state() != QAbstractSocket::UnconnectedState)
        return false;

    const QUrl url = QUrl::fromUserInput(d->m_networkAddress + QStringLiteral(":")
        + QString::number(d->m_networkPort));

    if (!url.isValid()) {
        setError(tr("Invalid connection settings for TCP communication specified."),
            QModbusDevice::ConnectionError);
        qCWarning(QT_MODBUS) << "(TCP client) Invalid host:" << url.host() << "or port:"
            << url.port();
        return false;
    }

    d->m_socket->connectToHost(url.host(), url.port());

    return true;
}

/*!
     \reimp
*/
void QModbusTcpClient::close()
{
    if (state() == QModbusDevice::UnconnectedState)
        return;

    Q_D(QModbusTcpClient);
    d->m_socket->disconnectFromHost();
}

QT_END_NAMESPACE
