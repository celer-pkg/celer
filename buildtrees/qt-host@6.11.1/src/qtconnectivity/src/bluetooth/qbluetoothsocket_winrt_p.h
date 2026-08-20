// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QBLUETOOTHSOCKET_WINRT_P_H
#define QBLUETOOTHSOCKET_WINRT_P_H

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

#include "qbluetoothsocket.h"
#include "qbluetoothsocketbase_p.h"
#include <QtGlobal>

QT_FORWARD_DECLARE_CLASS(SocketWorker)

namespace ABI {
    namespace Windows {
        namespace  Networking {
            struct IHostName;
        }
    }
}

namespace Microsoft {
    namespace WRL {
        template <typename T> class ComPtr;
    }
}
QT_BEGIN_NAMESPACE

class QBluetoothSocketPrivateWinRT final: public QBluetoothSocketBasePrivate
{
    Q_OBJECT
    friend class QBluetoothServerPrivate;

public:
    QBluetoothSocketPrivateWinRT();
    ~QBluetoothSocketPrivateWinRT() override;

    void connectToServiceHelper(const QBluetoothAddress &address,
                          quint16 port,
                          QIODevice::OpenMode openMode) override;

    void connectToService(const QBluetoothServiceInfo &service,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, const QBluetoothUuid &uuid,
                          QIODevice::OpenMode openMode) override;
    void connectToService(const QBluetoothAddress &address, quint16 port,
                          QIODevice::OpenMode openMode) override;

    bool ensureNativeSocket(QBluetoothServiceInfo::Protocol type) override;

    QString localName() const override;
    QBluetoothAddress localAddress() const override;
    quint16 localPort() const override;

    QString peerName() const override;
    QBluetoothAddress peerAddress() const override;
    quint16 peerPort() const override;

    void abort() override;
    void close() override;

    qint64 writeData(const char *data, qint64 maxSize) override;
    qint64 readData(char *data, qint64 maxSize) override;

    bool setSocketDescriptor(Microsoft::WRL::ComPtr<ABI::Windows::Networking::Sockets::IStreamSocket> socket,
                             QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::SocketState::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) override;

    bool setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                             QBluetoothSocket::SocketState socketState = QBluetoothSocket::SocketState::ConnectedState,
                             QBluetoothSocket::OpenMode openMode = QBluetoothSocket::ReadWrite) override;

    qint64 bytesAvailable() const override;
    bool canReadLine() const override;
    qint64 bytesToWrite() const override;

    SocketWorker *m_worker;

    Microsoft::WRL::ComPtr<ABI::Windows::Networking::Sockets::IStreamSocket> m_socketObject;
    Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IAsyncAction> m_connectOp;

    Q_INVOKABLE void addToPendingData(const QList<QByteArray> &data);

private slots:
    void handleNewData(const QList<QByteArray> &data);
    void handleError(QBluetoothSocket::SocketError error);

private:
    void connectToService(Microsoft::WRL::ComPtr<ABI::Windows::Networking::IHostName> hostName,
                          const QString &serviceName, QIODevice::OpenMode openMode);
    HRESULT handleConnectOpFinished(ABI::Windows::Foundation::IAsyncAction *action,
                                    ABI::Windows::Foundation::AsyncStatus status);

    QIODevice::OpenMode requestedOpenMode = QIODevice::NotOpen;
};

QT_END_NAMESPACE

#endif
