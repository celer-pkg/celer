// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSERVER_P_H
#define QBLUETOOTHSERVER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtGlobal>
#include <QList>
#include <QtBluetooth/QBluetoothSocket>
#include "qbluetoothserver.h"
#include "qbluetooth.h"

#if QT_CONFIG(bluez)
QT_FORWARD_DECLARE_CLASS(QSocketNotifier)
#endif

#ifdef QT_ANDROID_BLUETOOTH
#include <QtCore/QJniEnvironment>
#include <QtCore/QJniObject>
#include <QtBluetooth/QBluetoothUuid>

class ServerAcceptanceThread;
#endif

#ifdef QT_WINRT_BLUETOOTH
#include <QtCore/QMutex>

#include <wrl.h>
// No forward declares because QBluetoothServerPrivate::listener does not work with them
#include <windows.networking.sockets.h>
#endif

#ifdef QT_OSX_BLUETOOTH

#include "darwin/btdelegates_p.h"
#include "darwin/btraii_p.h"

#include <QtCore/QMutex>

#endif // QT_OSX_BLUETOOTH

QT_BEGIN_NAMESPACE

class QBluetoothAddress;
class QBluetoothSocket;
class QBluetoothServer;

class QBluetoothServerPrivate
#ifdef QT_OSX_BLUETOOTH
        : public DarwinBluetooth::SocketListener
#endif
{
    Q_DECLARE_PUBLIC(QBluetoothServer)

public:
    QBluetoothServerPrivate(QBluetoothServiceInfo::Protocol serverType, QBluetoothServer *parent);
    ~QBluetoothServerPrivate();

#if QT_CONFIG(bluez)
    void _q_newConnection();
    void setSocketSecurityLevel(QBluetooth::SecurityFlags requestedSecLevel, int *errnoCode);
    QBluetooth::SecurityFlags socketSecurityLevel() const;
    static QBluetoothSocket *createSocketForServer(
                QBluetoothServiceInfo::Protocol socketType = QBluetoothServiceInfo::RfcommProtocol);
#endif

public:
    QBluetoothSocket *socket = nullptr;

    int maxPendingConnections = 1;
    QBluetooth::SecurityFlags securityFlags = QBluetooth::Security::NoSecurity;
    QBluetoothServiceInfo::Protocol serverType;

protected:
    QBluetoothServer *q_ptr;

private:
    QBluetoothServer::Error m_lastError = QBluetoothServer::NoError;
#if QT_CONFIG(bluez)
    QSocketNotifier *socketNotifier = nullptr;
#elif defined(QT_ANDROID_BLUETOOTH)
    ServerAcceptanceThread *thread;
    QString m_serviceName;
    QBluetoothUuid m_uuid;
public:
    bool isListening() const;
    bool initiateActiveListening(const QBluetoothUuid& uuid, const QString &serviceName);
    bool deactivateActiveListening();
#elif defined(QT_WINRT_BLUETOOTH)
    EventRegistrationToken connectionToken {-1};

    mutable QMutex pendingConnectionsMutex;
    QList<Microsoft::WRL::ComPtr<ABI::Windows::Networking::Sockets::IStreamSocket>>
            pendingConnections;

    Microsoft::WRL::ComPtr<ABI::Windows::Networking::Sockets::IStreamSocketListener> socketListener;
    HRESULT handleClientConnection(ABI::Windows::Networking::Sockets::IStreamSocketListener *listener,
                                   ABI::Windows::Networking::Sockets::IStreamSocketListenerConnectionReceivedEventArgs *args);

public:
    bool isListening() const;
    Microsoft::WRL::ComPtr<ABI::Windows::Networking::Sockets::IStreamSocketListener> listener() { return socketListener; }
    bool initiateActiveListening(const QString &serviceName);
    bool deactivateActiveListening();
#endif

#ifdef QT_OSX_BLUETOOTH

public:

    friend class QBluetoothServer;
    friend class QBluetoothServiceInfoPrivate;

private:
    bool startListener(quint16 realPort);
    void stopListener();
    bool isListening() const;

    // SocketListener (delegate):
    void openNotifyRFCOMM(void *channel) override;
    void openNotifyL2CAP(void *channel) override;

    // Either a "temporary" channelID/PSM assigned by QBluetoothServer::listen,
    // or a real channelID/PSM returned by IOBluetooth after we've registered
    // a service.
    quint16 port;

    DarwinBluetooth::StrongReference listener;

    // These static functions below
    // deal with differences between bluetooth sockets
    // (bluez and QtBluetooth's API) and IOBluetooth, where it's not possible
    // to have a real PSM/channelID _before_ a service is registered,
    // the solution - "fake" ports.
    // These functions require external locking - using channelMapMutex.
    static QMutex &channelMapMutex();

    static bool channelIsBusy(quint16 channelID);
    static quint16 findFreeChannel();

    static bool psmIsBusy(quint16 psm);
    static quint16 findFreePSM();

    static void registerServer(QBluetoothServerPrivate *server, quint16 port);
    static QBluetoothServerPrivate *registeredServer(quint16 port, QBluetoothServiceInfo::Protocol protocol);
    static void unregisterServer(QBluetoothServerPrivate *server);

    using PendingConnection = DarwinBluetooth::StrongReference;
    QList<PendingConnection> pendingConnections;

#endif // QT_OSX_BLUETOOTH
};

QT_END_NAMESPACE

#endif
