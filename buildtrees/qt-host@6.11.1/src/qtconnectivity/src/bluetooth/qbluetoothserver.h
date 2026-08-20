// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSERVER_H
#define QBLUETOOTHSERVER_H

#include <QtBluetooth/qtbluetoothglobal.h>

#include <QtCore/QObject>

#include <QtBluetooth/QBluetoothAddress>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/QBluetoothSocket>
#include <QtBluetooth/QBluetoothServiceInfo>

QT_BEGIN_NAMESPACE

class QBluetoothServerPrivate;
class QBluetoothSocket;

class Q_BLUETOOTH_EXPORT QBluetoothServer : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError,
        UnknownError,
        PoweredOffError,
        InputOutputError,
        ServiceAlreadyRegisteredError,
        UnsupportedProtocolError,
        MissingPermissionsError
    };
    Q_ENUM(Error)

    explicit QBluetoothServer(QBluetoothServiceInfo::Protocol serverType, QObject *parent = nullptr);
    ~QBluetoothServer();

    void close();

    bool listen(const QBluetoothAddress &address = QBluetoothAddress(), quint16 port = 0);
    [[nodiscard]] QBluetoothServiceInfo listen(const QBluetoothUuid &uuid,
                                               const QString &serviceName = QString());
    bool isListening() const;

    void setMaxPendingConnections(int numConnections);
    int maxPendingConnections() const;

    bool hasPendingConnections() const;
    QBluetoothSocket *nextPendingConnection();

    QBluetoothAddress serverAddress() const;
    quint16 serverPort() const;

    void setSecurityFlags(QBluetooth::SecurityFlags security);
    QBluetooth::SecurityFlags securityFlags() const;

    QBluetoothServiceInfo::Protocol serverType() const;

    Error error() const;

Q_SIGNALS:
    void newConnection();
    void errorOccurred(QBluetoothServer::Error error);

protected:
    QBluetoothServerPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QBluetoothServer)
};

QT_END_NAMESPACE

#endif
