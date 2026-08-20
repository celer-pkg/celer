// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSOCKET_H
#define QBLUETOOTHSOCKET_H

#include <QtBluetooth/qtbluetoothglobal.h>

#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qbluetoothaddress.h>
#include <QtBluetooth/qbluetoothuuid.h>
#include <QtBluetooth/qbluetoothserviceinfo.h>

#include <QtCore/qiodevice.h>

QT_BEGIN_NAMESPACE


class QBluetoothSocketBasePrivate;

class Q_BLUETOOTH_EXPORT QBluetoothSocket : public QIODevice
{
    Q_OBJECT

    Q_DECLARE_PRIVATE(QBluetoothSocketBase)

    friend class QBluetoothServer;
    friend class QBluetoothServerPrivate;
    friend class QBluetoothSocketPrivateDarwin;
    friend class QBluetoothSocketPrivateAndroid;
    friend class QBluetoothSocketPrivateBluez;
    friend class QBluetoothSocketPrivateBluezDBus;
    friend class QBluetoothSocketPrivateDummy;
    friend class QBluetoothSocketPrivateWin;
    friend class QBluetoothSocketPrivateWinRT;

public:

    enum class SocketState {
        UnconnectedState,
        ServiceLookupState,
        ConnectingState,
        ConnectedState,
        BoundState,
        ClosingState,
        ListeningState
    };
    Q_ENUM(SocketState)

    enum class SocketError {
        NoSocketError,
        UnknownSocketError,
        RemoteHostClosedError,
        HostNotFoundError,
        ServiceNotFoundError,
        NetworkError,
        UnsupportedProtocolError,
        OperationError,
        MissingPermissionsError
    };
    Q_ENUM(SocketError)

    explicit QBluetoothSocket(QBluetoothServiceInfo::Protocol socketType, QObject *parent = nullptr);   // create socket of type socketType
    explicit QBluetoothSocket(QObject *parent = nullptr);  // create a blank socket
    virtual ~QBluetoothSocket();

    void abort();

    void close() override;

    bool isSequential() const override;

    qint64 bytesAvailable() const override;
    qint64 bytesToWrite() const override;

    bool canReadLine() const override;

    void connectToService(const QBluetoothServiceInfo &service, OpenMode openMode = ReadWrite);
    void connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid, OpenMode openMode = ReadWrite);
    void connectToService(const QBluetoothAddress &address, quint16 port, OpenMode openMode = ReadWrite);
    inline void connectToService(const QBluetoothAddress &address, QBluetoothUuid::ServiceClassUuid uuid,
                                 OpenMode mode = ReadWrite)
    {
        connectToService(address, QBluetoothUuid(uuid), mode);
    }
    void disconnectFromService();

    //bool flush();
    //bool isValid() const;

    QString localName() const;
    QBluetoothAddress localAddress() const;
    quint16 localPort() const;

    QString peerName() const;
    QBluetoothAddress peerAddress() const;
    quint16 peerPort() const;
    //QBluetoothServiceInfo peerService() const;

    //qint64 readBufferSize() const;
    //void setReadBufferSize(qint64 size);

    bool setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                             SocketState socketState = SocketState::ConnectedState,
                             OpenMode openMode = ReadWrite);
    int socketDescriptor() const;

    QBluetoothServiceInfo::Protocol socketType() const;
    SocketState state() const;
    SocketError error() const;
    QString errorString() const;

    //bool waitForConnected(int msecs = 30000);
    //bool waitForDisconnected(int msecs = 30000);
    //virtual bool waitForReadyRead(int msecs = 30000);

    void setPreferredSecurityFlags(QBluetooth::SecurityFlags flags);
    QBluetooth::SecurityFlags preferredSecurityFlags() const;

Q_SIGNALS:
    void connected();
    void disconnected();
    void errorOccurred(QBluetoothSocket::SocketError error);
    void stateChanged(QBluetoothSocket::SocketState state);

protected:
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

    void setSocketState(SocketState state);
    void setSocketError(SocketError error);

    void doDeviceDiscovery(const QBluetoothServiceInfo &service, OpenMode openMode);

private Q_SLOTS:
    void serviceDiscovered(const QBluetoothServiceInfo &service);
    void discoveryFinished();


protected:
#if QT_CONFIG(bluez)
    //evil hack to enable QBluetoothServer on Bluez to set the desired d_ptr
    explicit QBluetoothSocket(QBluetoothSocketBasePrivate *d,
                              QBluetoothServiceInfo::Protocol socketType,
                              QObject *parent = nullptr);
#endif

    QBluetoothSocketBasePrivate *d_ptr;

private:
    friend class QLowEnergyControllerPrivateBluez;
};


QT_END_NAMESPACE

#endif
