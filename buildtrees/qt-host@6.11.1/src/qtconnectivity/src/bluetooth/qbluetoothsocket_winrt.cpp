// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothsocket_winrt_p.h"
#include "qbluetoothutils_winrt_p.h"

#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/qbluetoothdeviceinfo.h>
#include <QtBluetooth/qbluetoothserviceinfo.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/QPointer>
#include <QtCore/private/qfunctions_winrt_p.h>

#include <robuffer.h>
#include <windows.devices.bluetooth.h>
#include <windows.networking.sockets.h>
#include <windows.storage.streams.h>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Bluetooth::Rfcomm;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Networking;
using namespace ABI::Windows::Networking::Sockets;
using namespace ABI::Windows::Storage::Streams;

typedef IAsyncOperationWithProgressCompletedHandler<IBuffer *, UINT32> SocketReadCompletedHandler;
typedef IAsyncOperationWithProgress<IBuffer *, UINT32> IAsyncBufferOperation;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

struct SocketGlobal
{
    SocketGlobal()
    {
        HRESULT hr;
        hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(),
            &bufferFactory);
        Q_ASSERT_SUCCEEDED(hr);
    }

    ComPtr<IBufferFactory> bufferFactory;
};
Q_GLOBAL_STATIC(SocketGlobal, g)

#define READ_BUFFER_SIZE 65536

static inline QString qt_QStringFromHString(const HString &string)
{
    UINT32 length;
    PCWSTR rawString = string.GetRawBuffer(&length);
    if (length > INT_MAX)
        length = INT_MAX;
    return QString::fromWCharArray(rawString, int(length));
}

static qint64 writeIOStream(ComPtr<IOutputStream> stream, const char *data, qint64 len)
{
    ComPtr<IBuffer> tempBuffer;
    if (len > UINT32_MAX) {
        qCWarning(QT_BT_WINDOWS) << "writeIOStream can only write up to" << UINT32_MAX << "bytes.";
        len = UINT32_MAX;
    }
    quint32 ulen = static_cast<quint32>(len);
    HRESULT hr = g->bufferFactory->Create(ulen, &tempBuffer);
    Q_ASSERT_SUCCEEDED(hr);
    hr = tempBuffer->put_Length(ulen);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteArrayAccess;
    hr = tempBuffer.As(&byteArrayAccess);
    Q_ASSERT_SUCCEEDED(hr);
    byte *bytes;
    hr = byteArrayAccess->Buffer(&bytes);
    Q_ASSERT_SUCCEEDED(hr);
    memcpy(bytes, data, ulen);
    ComPtr<IAsyncOperationWithProgress<UINT32, UINT32>> op;
    hr = stream->WriteAsync(tempBuffer.Get(), &op);
    RETURN_IF_FAILED("Failed to write to stream", return -1);
    UINT32 bytesWritten;
    hr = QWinRTFunctions::await(op, &bytesWritten);
    RETURN_IF_FAILED("Failed to write to stream", return -1);
    return bytesWritten;
}

class SocketWorker : public QObject
{
    Q_OBJECT
public:
    SocketWorker()
    {
    }

    ~SocketWorker()
    {
    }
    void close()
    {
        m_shuttingDown = true;
        if (m_readOp) {
            onReadyRead(m_readOp.Get(), Canceled);
            ComPtr<IAsyncInfo> info;
            HRESULT hr = m_readOp.As(&info);
            Q_ASSERT_SUCCEEDED(hr);
            if (info) {
                hr = info->Cancel();
                Q_ASSERT_SUCCEEDED(hr);
                hr = info->Close();
                Q_ASSERT_SUCCEEDED(hr);
            }
            m_readOp.Reset();
        }
    }

signals:
    void newDataReceived(const QList<QByteArray> &data);
    void socketErrorOccured(QBluetoothSocket::SocketError error);

public slots:
    Q_INVOKABLE void notifyAboutNewData()
    {
        const QList<QByteArray> newData = std::move(m_pendingData);
        m_pendingData.clear();
        emit newDataReceived(newData);
    }

public:
    void startReading()
    {
        ComPtr<IBuffer> tempBuffer;
        HRESULT hr = g->bufferFactory->Create(READ_BUFFER_SIZE, &tempBuffer);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<IInputStream> stream;
        hr = m_socket->get_InputStream(&stream);
        Q_ASSERT_SUCCEEDED(hr);
        hr = stream->ReadAsync(tempBuffer.Get(), READ_BUFFER_SIZE, InputStreamOptions_Partial, m_readOp.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);
        QPointer<SocketWorker> thisPtr(this);
        hr = m_readOp->put_Completed(
                Callback<SocketReadCompletedHandler>([thisPtr](IAsyncBufferOperation *asyncInfo,
                                                               AsyncStatus status) {
                    if (thisPtr)
                        return thisPtr->onReadyRead(asyncInfo, status);
                    return S_OK;
                }).Get());
        Q_ASSERT_SUCCEEDED(hr);
    }

    HRESULT onReadyRead(IAsyncBufferOperation *asyncInfo, AsyncStatus status)
    {
        if (m_shuttingDown)
            return S_OK;

        if (asyncInfo == m_readOp.Get())
            m_readOp.Reset();
        else
            Q_ASSERT(false);

        // A read in UnconnectedState will close the socket and return -1 and thus tell the caller,
        // that the connection was closed. The socket cannot be closed here, as the subsequent read
        // might fail then.
        if (status == Error || status == Canceled) {
            emit socketErrorOccured(QBluetoothSocket::SocketError::NetworkError);
            return S_OK;
        }

        ComPtr<IBuffer> tempBuffer;
        HRESULT hr = asyncInfo->GetResults(&tempBuffer);
        if (FAILED(hr)) {
            qErrnoWarning(hr, "Failed to get read results buffer");
            emit socketErrorOccured(QBluetoothSocket::SocketError::UnknownSocketError);
            return S_OK;
        }

        UINT32 bufferLength;
        hr = tempBuffer->get_Length(&bufferLength);
        if (FAILED(hr)) {
            qErrnoWarning(hr, "Failed to get buffer length");
            emit socketErrorOccured(QBluetoothSocket::SocketError::UnknownSocketError);
            return S_OK;
        }
        // A zero sized buffer length signals, that the remote host closed the connection. The socket
        // cannot be closed though, as the following read might have socket descriptor -1 and thus and
        // the closing of the socket won't be communicated to the caller. So only the error is set. The
        // actual socket close happens inside of read.
        if (!bufferLength) {
            emit socketErrorOccured(QBluetoothSocket::SocketError::RemoteHostClosedError);
            return S_OK;
        }

        ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteArrayAccess;
        hr = tempBuffer.As(&byteArrayAccess);
        if (FAILED(hr)) {
            qErrnoWarning(hr, "Failed to get cast buffer");
            emit socketErrorOccured(QBluetoothSocket::SocketError::UnknownSocketError);
            return S_OK;
        }
        byte *data;
        hr = byteArrayAccess->Buffer(&data);
        if (FAILED(hr)) {
            qErrnoWarning(hr, "Failed to access buffer data");
            emit socketErrorOccured(QBluetoothSocket::SocketError::UnknownSocketError);
            return S_OK;
        }

        QByteArray newData(reinterpret_cast<const char*>(data), int(bufferLength));
        if (m_pendingData.isEmpty())
            QMetaObject::invokeMethod(this, "notifyAboutNewData", Qt::QueuedConnection);
        m_pendingData << newData;

        UINT32 readBufferLength;
        ComPtr<IInputStream> stream;
        hr = m_socket->get_InputStream(&stream);
        if (FAILED(hr)) {
            qErrnoWarning(hr, "Failed to obtain input stream");
            emit socketErrorOccured(QBluetoothSocket::SocketError::UnknownSocketError);
            return S_OK;
        }

        // Reuse the stream buffer
        hr = tempBuffer->get_Capacity(&readBufferLength);
        if (FAILED(hr)) {
            qErrnoWarning(hr, "Failed to get buffer capacity");
            emit socketErrorOccured(QBluetoothSocket::SocketError::UnknownSocketError);
            return S_OK;
        }
        hr = tempBuffer->put_Length(0);
        if (FAILED(hr)) {
            qErrnoWarning(hr, "Failed to set buffer length");
            emit socketErrorOccured(QBluetoothSocket::SocketError::UnknownSocketError);
            return S_OK;
        }

        hr = stream->ReadAsync(tempBuffer.Get(), readBufferLength, InputStreamOptions_Partial, &m_readOp);
        if (FAILED(hr)) {
            qErrnoWarning(hr, "onReadyRead(): Could not read into socket stream buffer.");
            emit socketErrorOccured(QBluetoothSocket::SocketError::UnknownSocketError);
            return S_OK;
        }
        QPointer<SocketWorker> thisPtr(this);
        hr = m_readOp->put_Completed(
                Callback<SocketReadCompletedHandler>([thisPtr](IAsyncBufferOperation *asyncInfo,
                                                               AsyncStatus status) {
                    if (thisPtr)
                        return thisPtr->onReadyRead(asyncInfo, status);
                    return S_OK;
                }).Get());
        if (FAILED(hr)) {
            qErrnoWarning(hr, "onReadyRead(): Failed to set socket read callback.");
            emit socketErrorOccured(QBluetoothSocket::SocketError::UnknownSocketError);
            return S_OK;
        }
        return S_OK;
    }

    void setSocket(ComPtr<IStreamSocket> socket) { m_socket = socket; }

private:
    ComPtr<IStreamSocket> m_socket;
    QList<QByteArray> m_pendingData;
    bool m_shuttingDown = false;

    ComPtr<IAsyncOperationWithProgress<IBuffer *, UINT32>> m_readOp;
};

QBluetoothSocketPrivateWinRT::QBluetoothSocketPrivateWinRT()
    : m_worker(new SocketWorker())
{
    mainThreadCoInit(this);
    secFlags = QBluetooth::Security::NoSecurity;
    connect(m_worker, &SocketWorker::newDataReceived,
            this, &QBluetoothSocketPrivateWinRT::handleNewData, Qt::QueuedConnection);
    connect(m_worker, &SocketWorker::socketErrorOccured,
            this, &QBluetoothSocketPrivateWinRT::handleError, Qt::QueuedConnection);
}

QBluetoothSocketPrivateWinRT::~QBluetoothSocketPrivateWinRT()
{
    abort();
    mainThreadCoUninit(this);
}

bool QBluetoothSocketPrivateWinRT::ensureNativeSocket(QBluetoothServiceInfo::Protocol type)
{
    if (socket != -1) {
        if (type == socketType)
            return true;
        m_socketObject = nullptr;
        socket = -1;
    }
    socketType = type;
    if (socketType != QBluetoothServiceInfo::RfcommProtocol)
        return false;

    HRESULT hr;
    hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Networking_Sockets_StreamSocket).Get(), &m_socketObject);
    if (FAILED(hr) || !m_socketObject) {
        qErrnoWarning(hr, "ensureNativeSocket: Could not create socket instance");
        return false;
    }
    socket = qintptr(m_socketObject.Get());
    m_worker->setSocket(m_socketObject);

    return true;
}

void QBluetoothSocketPrivateWinRT::connectToService(Microsoft::WRL::ComPtr<IHostName> hostName,
                                                    const QString &serviceName,
                                                    QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (socket == -1 && !ensureNativeSocket(socketType)) {
        errorString = QBluetoothSocket::tr("Unknown socket error");
        q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
        return;
    }

    HStringReference serviceNameReference(reinterpret_cast<LPCWSTR>(serviceName.utf16()));

    HRESULT hr = m_socketObject->ConnectAsync(hostName.Get(), serviceNameReference.Get(), &m_connectOp);
    if (hr == E_ACCESSDENIED) {
        qErrnoWarning(hr, "QBluetoothSocketPrivateWinRT::connectToService: Unable to connect to bluetooth socket."
            "Please check your manifest capabilities.");
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return;
    }
    Q_ASSERT_SUCCEEDED(hr);

    q->setSocketState(QBluetoothSocket::SocketState::ConnectingState);
    requestedOpenMode = openMode;
    hr = m_connectOp->put_Completed(Callback<IAsyncActionCompletedHandler>(
                                     this, &QBluetoothSocketPrivateWinRT::handleConnectOpFinished).Get());
    RETURN_VOID_IF_FAILED("connectToHostByName: Could not register \"connectOp\" callback");
    return;
}

void QBluetoothSocketPrivateWinRT::connectToServiceHelper(const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (socket == -1 && !ensureNativeSocket(socketType)) {
        errorString = QBluetoothSocket::tr("Unknown socket error");
        q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
        return;
    }

    const QString addressString = address.toString();
    HStringReference hostNameRef(reinterpret_cast<LPCWSTR>(addressString.utf16()));
    ComPtr<IHostNameFactory> hostNameFactory;
    HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Networking_HostName).Get(),
                                      &hostNameFactory);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IHostName> remoteHost;
    hr = hostNameFactory->CreateHostName(hostNameRef.Get(), &remoteHost);
    RETURN_VOID_IF_FAILED("QBluetoothSocketPrivateWinRT::connectToService: Could not create hostname.");
    const QString portString = QString::number(port);
    connectToService(remoteHost, portString, openMode);
}

void QBluetoothSocketPrivateWinRT::connectToService(
        const QBluetoothServiceInfo &service, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::SocketState::UnconnectedState
            && q->state() != QBluetoothSocket::SocketState::ServiceLookupState) {
        qCWarning(QT_BT_WINDOWS) << "QBluetoothSocket::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    // we are checking the service protocol and not socketType()
    // socketType will change in ensureNativeSocket()
    if (service.socketProtocol() != QBluetoothServiceInfo::RfcommProtocol) {
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    const QString connectionHostName = service.attribute(0xBEEF).toString();
    const QString connectionServiceName = service.attribute(0xBEF0).toString();
    if (service.protocolServiceMultiplexer() > 0) {
        Q_ASSERT(service.socketProtocol() == QBluetoothServiceInfo::L2capProtocol);

       if (!ensureNativeSocket(QBluetoothServiceInfo::L2capProtocol)) {
           errorString = QBluetoothSocket::tr("Unknown socket error");
           q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
           return;
       }
       connectToServiceHelper(service.device().address(),
                              quint16(service.protocolServiceMultiplexer()), openMode);
    } else if (!connectionHostName.isEmpty() && !connectionServiceName.isEmpty()) {
        Q_ASSERT(service.socketProtocol() == QBluetoothServiceInfo::RfcommProtocol);
        if (!ensureNativeSocket(QBluetoothServiceInfo::RfcommProtocol)) {
            errorString = QBluetoothSocket::tr("Unknown socket error");
            q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
            return;
        }
        HStringReference hostNameRef(reinterpret_cast<LPCWSTR>(connectionHostName.utf16()));
        ComPtr<IHostNameFactory> hostNameFactory;
        HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Networking_HostName).Get(),
                                          &hostNameFactory);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<IHostName> remoteHost;
        hr = hostNameFactory->CreateHostName(hostNameRef.Get(), &remoteHost);
        connectToService(remoteHost, connectionServiceName, openMode);
    } else if (service.serverChannel() > 0) {
        Q_ASSERT(service.socketProtocol() == QBluetoothServiceInfo::RfcommProtocol);

       if (!ensureNativeSocket(QBluetoothServiceInfo::RfcommProtocol)) {
           errorString = QBluetoothSocket::tr("Unknown socket error");
           q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
           return;
       }
       connectToServiceHelper(service.device().address(), quint16(service.serverChannel()),
                              openMode);
    } else {
       // try doing service discovery to see if we can find the socket
       if (service.serviceUuid().isNull()
               && !service.serviceClassUuids().contains(QBluetoothUuid::ServiceClassUuid::SerialPort)) {
           qCWarning(QT_BT_WINDOWS) << "No port, no PSM, and no UUID provided. Unable to connect";
           return;
       }
       qCDebug(QT_BT_WINDOWS) << "Need a port/psm, doing discovery";
       q->doDeviceDiscovery(service, openMode);
   }
}

void QBluetoothSocketPrivateWinRT::connectToService(
        const QBluetoothAddress &address, const QBluetoothUuid &uuid, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::SocketState::UnconnectedState) {
        qCWarning(QT_BT_WINDOWS) << "QBluetoothSocketPrivateWinRT::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    if (q->socketType() != QBluetoothServiceInfo::RfcommProtocol) {
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

    QBluetoothServiceInfo service;
    QBluetoothDeviceInfo device(address, QString(), QBluetoothDeviceInfo::MiscellaneousDevice);
    service.setDevice(device);
    service.setServiceUuid(uuid);
    q->doDeviceDiscovery(service, openMode);
}

void QBluetoothSocketPrivateWinRT::connectToService(
        const QBluetoothAddress &address, quint16 port, QIODevice::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);

    if (q->state() != QBluetoothSocket::SocketState::UnconnectedState) {
        qCWarning(QT_BT_WINDOWS) << "QBluetoothSocketPrivateWinRT::connectToService called on busy socket";
        errorString = QBluetoothSocket::tr("Trying to connect while connection is in progress");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return;
    }

    if (q->socketType() != QBluetoothServiceInfo::RfcommProtocol) {
        errorString = QBluetoothSocket::tr("Socket type not supported");
        q->setSocketError(QBluetoothSocket::SocketError::UnsupportedProtocolError);
        return;
    }

     connectToServiceHelper(address, port, openMode);
}

void QBluetoothSocketPrivateWinRT::abort()
{
    Q_Q(QBluetoothSocket);
    if (state == QBluetoothSocket::SocketState::UnconnectedState)
        return;

    disconnect(m_worker, &SocketWorker::newDataReceived,
        this, &QBluetoothSocketPrivateWinRT::handleNewData);
    disconnect(m_worker, &SocketWorker::socketErrorOccured,
        this, &QBluetoothSocketPrivateWinRT::handleError);
    m_worker->close();
    m_worker->deleteLater();

    if (socket != -1) {
        m_socketObject = nullptr;
        socket = -1;
    }

    const bool wasConnected = q->state() == QBluetoothSocket::SocketState::ConnectedState;
    q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
    if (wasConnected) {
        q->setOpenMode(QIODevice::NotOpen);
        emit q->readChannelFinished();
    }
}

QString QBluetoothSocketPrivateWinRT::localName() const
{
    const QBluetoothAddress address = localAddress();
    if (address.isNull())
        return QString();

    QBluetoothLocalDevice device(address);
    return device.name();
}

static QString fromWinApiAddress(HString address)
{
    // WinAPI returns address with parentheses around it. We need to remove
    // them to convert to QBluetoothAddress.
    QString addressStr(qt_QStringFromHString(address));
    if (addressStr.startsWith(QLatin1Char('(')) && addressStr.endsWith(QLatin1Char(')'))) {
        addressStr = addressStr.sliced(1, addressStr.size() - 2);
    }
    return addressStr;
}

QBluetoothAddress QBluetoothSocketPrivateWinRT::localAddress() const
{
    if (!m_socketObject)
        return QBluetoothAddress();

    HRESULT hr;
    ComPtr<IStreamSocketInformation> info;
    hr = m_socketObject->get_Information(&info);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IHostName> localHost;
    hr = info->get_LocalAddress(&localHost);
    Q_ASSERT_SUCCEEDED(hr);
    if (localHost) {
        HString localAddress;
        hr = localHost->get_CanonicalName(localAddress.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);
        return QBluetoothAddress(fromWinApiAddress(std::move(localAddress)));
    }
    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivateWinRT::localPort() const
{
    if (!m_socketObject)
        return 0;

    HRESULT hr;
    ComPtr<IStreamSocketInformation> info;
    hr = m_socketObject->get_Information(&info);
    Q_ASSERT_SUCCEEDED(hr);
    HString localPortString;
    hr = info->get_LocalPort(localPortString.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);
    bool ok = true;
    const uint port = qt_QStringFromHString(localPortString).toUInt(&ok);
    if (!ok || port > UINT16_MAX) {
        qCWarning(QT_BT_WINDOWS) << "Unexpected local port";
        return 0;
    }
    return quint16(port);
}

QString QBluetoothSocketPrivateWinRT::peerName() const
{
    if (!m_socketObject)
        return QString();

    HRESULT hr;
    ComPtr<IStreamSocketInformation> info;
    hr = m_socketObject->get_Information(&info);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IHostName> remoteHost;
    hr = info->get_RemoteHostName(&remoteHost);
    Q_ASSERT_SUCCEEDED(hr);
    if (remoteHost) {
        HString remoteHostName;
        hr = remoteHost->get_DisplayName(remoteHostName.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);
        return qt_QStringFromHString(remoteHostName);
    }
    return {};
}

QBluetoothAddress QBluetoothSocketPrivateWinRT::peerAddress() const
{
    if (!m_socketObject)
        return QBluetoothAddress();

    HRESULT hr;
    ComPtr<IStreamSocketInformation> info;
    hr = m_socketObject->get_Information(&info);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IHostName> remoteHost;
    hr = info->get_RemoteAddress(&remoteHost);
    Q_ASSERT_SUCCEEDED(hr);
    if (remoteHost) {
        HString remoteAddress;
        hr = remoteHost->get_CanonicalName(remoteAddress.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);
        return QBluetoothAddress(fromWinApiAddress(std::move(remoteAddress)));
    }
    return QBluetoothAddress();
}

quint16 QBluetoothSocketPrivateWinRT::peerPort() const
{
    if (!m_socketObject)
        return 0;

    HRESULT hr;
    ComPtr<IStreamSocketInformation> info;
    hr = m_socketObject->get_Information(&info);
    Q_ASSERT_SUCCEEDED(hr);
    HString remotePortString;
    hr = info->get_RemotePort(remotePortString.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);
    bool ok = true;
    const uint port = qt_QStringFromHString(remotePortString).toUInt(&ok);
    if (!ok || port > UINT16_MAX) {
        qCWarning(QT_BT_WINDOWS) << "Unexpected remote port";
        return 0;
    }
    return quint16(port);
}

qint64 QBluetoothSocketPrivateWinRT::writeData(const char *data, qint64 maxSize)
{
    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot write while not connected");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    ComPtr<IOutputStream> stream;
    HRESULT hr;
    hr = m_socketObject->get_OutputStream(&stream);
    Q_ASSERT_SUCCEEDED(hr);

    qint64 bytesWritten = writeIOStream(stream, data, maxSize);
    if (bytesWritten < 0) {
        qCWarning(QT_BT_WINDOWS) << "Socket::writeData: " << state;
        errorString = QBluetoothSocket::tr("Cannot read while not connected");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
    }

    emit q->bytesWritten(bytesWritten);
    return bytesWritten;
}

qint64 QBluetoothSocketPrivateWinRT::readData(char *data, qint64 maxSize)
{
    Q_Q(QBluetoothSocket);

    if (state != QBluetoothSocket::SocketState::ConnectedState) {
        errorString = QBluetoothSocket::tr("Cannot read while not connected");
        q->setSocketError(QBluetoothSocket::SocketError::OperationError);
        return -1;
    }

    if (!rxBuffer.isEmpty()) {
        if (maxSize > INT_MAX)
            maxSize = INT_MAX;
        return rxBuffer.read(data, int(maxSize));
    }

    return 0;
}

void QBluetoothSocketPrivateWinRT::close()
{
    abort();
}

bool QBluetoothSocketPrivateWinRT::setSocketDescriptor(int socketDescriptor, QBluetoothServiceInfo::Protocol socketType,
                                           QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_UNUSED(socketDescriptor);
    Q_UNUSED(socketType);
    Q_UNUSED(socketState);
    Q_UNUSED(openMode);
    qCWarning(QT_BT_WINDOWS) << "No socket descriptor support on WinRT.";
    return false;
}

bool QBluetoothSocketPrivateWinRT::setSocketDescriptor(ComPtr<IStreamSocket> socketPtr, QBluetoothServiceInfo::Protocol socketType,
                                           QBluetoothSocket::SocketState socketState, QBluetoothSocket::OpenMode openMode)
{
    Q_Q(QBluetoothSocket);
    if (socketType != QBluetoothServiceInfo::RfcommProtocol || !socketPtr)
        return false;

    m_socketObject = socketPtr;
    socket = qintptr(m_socketObject.Get());
    m_worker->setSocket(m_socketObject);
    q->setSocketState(socketState);
    if (socketState == QBluetoothSocket::SocketState::ConnectedState)
        m_worker->startReading();
    // QBluetoothSockets are unbuffered on Windows
    q->setOpenMode(openMode | QIODevice::Unbuffered);
    return true;
}

qint64 QBluetoothSocketPrivateWinRT::bytesAvailable() const
{
    return rxBuffer.size();
}

qint64 QBluetoothSocketPrivateWinRT::bytesToWrite() const
{
    return 0; // nothing because always unbuffered
}

bool QBluetoothSocketPrivateWinRT::canReadLine() const
{
    return rxBuffer.canReadLine();
}

void QBluetoothSocketPrivateWinRT::handleNewData(const QList<QByteArray> &data)
{
    // Defer putting the data into the list until the next event loop iteration
    // (where the readyRead signal is emitted as well)
    QMetaObject::invokeMethod(this, "addToPendingData", Qt::QueuedConnection,
                              Q_ARG(QList<QByteArray>, data));
}

void QBluetoothSocketPrivateWinRT::handleError(QBluetoothSocket::SocketError error)
{
    Q_Q(QBluetoothSocket);
    switch (error) {
    case QBluetoothSocket::SocketError::NetworkError:
        errorString = QBluetoothSocket::tr("Network error");
        break;
    case QBluetoothSocket::SocketError::RemoteHostClosedError:
        errorString = QBluetoothSocket::tr("Remote host closed connection");
        break;
    default:
        errorString = QBluetoothSocket::tr("Unknown socket error");
    }

    q->setSocketError(error);
    const bool wasConnected = q->state() == QBluetoothSocket::SocketState::ConnectedState;
    q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
    if (wasConnected) {
        q->setOpenMode(QIODevice::NotOpen);
        emit q->readChannelFinished();
    }
}

void QBluetoothSocketPrivateWinRT::addToPendingData(const QList<QByteArray> &data)
{
    Q_Q(QBluetoothSocket);
    for (const QByteArray &newData : data) {
        char *writePointer = rxBuffer.reserve(newData.length());
        memcpy(writePointer, newData.data(), size_t(newData.length()));
    }
    emit q->readyRead();
}

HRESULT QBluetoothSocketPrivateWinRT::handleConnectOpFinished(ABI::Windows::Foundation::IAsyncAction *action, ABI::Windows::Foundation::AsyncStatus status)
{
    Q_Q(QBluetoothSocket);
    if (status != Completed || !m_connectOp) { // Protect against a late callback
        errorString = QBluetoothSocket::tr("Unknown socket error");
        q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return S_OK;
    }

    HRESULT hr = action->GetResults();
    switch (hr) {

    // A connection attempt failed because the connected party did not properly respond after a
    // period of time, or established connection failed because connected host has failed to respond.
    case HRESULT_FROM_WIN32(WSAETIMEDOUT):
        errorString = QBluetoothSocket::tr("Connection timed out");
        q->setSocketError(QBluetoothSocket::SocketError::NetworkError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return S_OK;
    // A socket operation was attempted to an unreachable host.
    case HRESULT_FROM_WIN32(WSAEHOSTUNREACH):
        errorString = QBluetoothSocket::tr("Host not reachable");
        q->setSocketError(QBluetoothSocket::SocketError::HostNotFoundError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return S_OK;
    // No connection could be made because the target machine actively refused it.
    case HRESULT_FROM_WIN32(WSAECONNREFUSED):
        errorString = QBluetoothSocket::tr("Host refused connection");
        q->setSocketError(QBluetoothSocket::SocketError::HostNotFoundError);
        q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
        return S_OK;
    default:
        if (FAILED(hr)) {
            errorString = QBluetoothSocket::tr("Unknown socket error");
            q->setSocketError(QBluetoothSocket::SocketError::UnknownSocketError);
            q->setSocketState(QBluetoothSocket::SocketState::UnconnectedState);
            return S_OK;
        }
    }

    // The callback might be triggered several times if we do not cancel/reset it here
    if (m_connectOp) {
        ComPtr<IAsyncInfo> info;
        hr = m_connectOp.As(&info);
        Q_ASSERT_SUCCEEDED(hr);
        if (info) {
            hr = info->Cancel();
            Q_ASSERT_SUCCEEDED(hr);
            hr = info->Close();
            Q_ASSERT_SUCCEEDED(hr);
        }
        m_connectOp.Reset();
    }

    // QBluetoothSockets are unbuffered on Windows
    q->setOpenMode(requestedOpenMode | QIODevice::Unbuffered);
    q->setSocketState(QBluetoothSocket::SocketState::ConnectedState);
    m_worker->startReading();

    return S_OK;
}

QT_END_NAMESPACE

#include "qbluetoothsocket_winrt.moc"
