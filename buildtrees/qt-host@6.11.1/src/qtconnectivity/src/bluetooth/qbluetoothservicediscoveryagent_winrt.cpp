// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothservicediscoveryagent_p.h"
#include "qbluetoothutils_winrt_p.h"

#include <QtCore/QLoggingCategory>
#include <QtCore/private/qfunctions_winrt_p.h>

#include <functional>
#include <robuffer.h>
#include <windows.devices.enumeration.h>
#include <windows.devices.bluetooth.h>
#include <windows.foundation.collections.h>
#include <windows.networking.h>
#include <windows.storage.streams.h>
#include <wrl.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Devices;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Bluetooth::Rfcomm;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Storage::Streams;

typedef Collections::IKeyValuePair<UINT32, IBuffer *> ValueItem;
typedef Collections::IIterable<ValueItem *> ValueIterable;
typedef Collections::IIterator<ValueItem *> ValueIterator;

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

#define TYPE_UINT8 8
#define TYPE_UINT16 9
#define TYPE_UINT32 10
#define TYPE_SHORT_UUID 25
#define TYPE_LONG_UUID 28
#define TYPE_STRING 37
#define TYPE_SEQUENCE 53

// Helper to reverse given uchar array
static void reverseArray(uchar data[], size_t length)
{
    for (size_t i = length; i > length/2; i--)
        std::swap(data[length - i], data[i - 1]);
}

class QWinRTBluetoothServiceDiscoveryWorker : public QObject
{
    Q_OBJECT
public:
    explicit QWinRTBluetoothServiceDiscoveryWorker(quint64 targetAddress,
                                                   QBluetoothServiceDiscoveryAgent::DiscoveryMode mode);
    ~QWinRTBluetoothServiceDiscoveryWorker();
    void start();

Q_SIGNALS:
    void serviceFound(quint64 deviceAddress, const QBluetoothServiceInfo &info);
    void scanFinished(quint64 deviceAddress);
    void errorOccured();

private:
    HRESULT onBluetoothDeviceFoundAsync(IAsyncOperation<BluetoothDevice *> *op, AsyncStatus status);

    void processServiceSearchResult(quint64 address, ComPtr<IVectorView<RfcommDeviceService*>> services);
    QBluetoothServiceInfo::Sequence readSequence(ComPtr<IDataReader> dataReader, bool *ok, quint8 *bytesRead, int depth = 0);

private:
    quint64 m_targetAddress;
    QBluetoothServiceDiscoveryAgent::DiscoveryMode m_mode;
};

QWinRTBluetoothServiceDiscoveryWorker::QWinRTBluetoothServiceDiscoveryWorker(quint64 targetAddress,
                                                                             QBluetoothServiceDiscoveryAgent::DiscoveryMode mode)
    : m_targetAddress(targetAddress)
    , m_mode(mode)
{
}

QWinRTBluetoothServiceDiscoveryWorker::~QWinRTBluetoothServiceDiscoveryWorker()
{
}

void QWinRTBluetoothServiceDiscoveryWorker::start()
{
    ComPtr<IBluetoothDeviceStatics> deviceStatics;
    HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothDevice).Get(), &deviceStatics);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IAsyncOperation<BluetoothDevice *>> deviceFromAddressOperation;
    hr = deviceStatics->FromBluetoothAddressAsync(m_targetAddress, &deviceFromAddressOperation);
    Q_ASSERT_SUCCEEDED(hr);
    hr = deviceFromAddressOperation->put_Completed(Callback<IAsyncOperationCompletedHandler<BluetoothDevice *>>
                                              (this, &QWinRTBluetoothServiceDiscoveryWorker::onBluetoothDeviceFoundAsync).Get());
    Q_ASSERT_SUCCEEDED(hr);
}

HRESULT QWinRTBluetoothServiceDiscoveryWorker::onBluetoothDeviceFoundAsync(IAsyncOperation<BluetoothDevice *> *op, AsyncStatus status)
{
    if (status != Completed) {
        qCDebug(QT_BT_WINDOWS) << "Could not find device";
        emit errorOccured();
        return S_OK;
    }

    ComPtr<IBluetoothDevice> device;
    HRESULT hr;
    hr = op->GetResults(&device);
    Q_ASSERT_SUCCEEDED(hr);
    quint64 address;
    device->get_BluetoothAddress(&address);

    ComPtr<IBluetoothDevice3> device3;
    hr = device.As(&device3);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IAsyncOperation<RfcommDeviceServicesResult *>> serviceOp;
    const BluetoothCacheMode cacheMode = m_mode == QBluetoothServiceDiscoveryAgent::MinimalDiscovery
        ? BluetoothCacheMode_Cached : BluetoothCacheMode_Uncached;
    hr = device3->GetRfcommServicesWithCacheModeAsync(cacheMode, &serviceOp);
    Q_ASSERT_SUCCEEDED(hr);
    hr = serviceOp->put_Completed(Callback<IAsyncOperationCompletedHandler<RfcommDeviceServicesResult *>>
        ([address, this](IAsyncOperation<RfcommDeviceServicesResult *> *op, AsyncStatus status)
    {
        if (status != Completed) {
            qCDebug(QT_BT_WINDOWS) << "Could not obtain service list";
            emit errorOccured();
            return S_OK;
        }

        ComPtr<IRfcommDeviceServicesResult> result;
        HRESULT hr = op->GetResults(&result);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<IVectorView<RfcommDeviceService*>> commServices;
        hr = result->get_Services(&commServices);
        Q_ASSERT_SUCCEEDED(hr);
        processServiceSearchResult(address, commServices);
        return S_OK;
    }).Get());
    Q_ASSERT_SUCCEEDED(hr);

    return S_OK;
}

void QWinRTBluetoothServiceDiscoveryWorker::processServiceSearchResult(quint64 address, ComPtr<IVectorView<RfcommDeviceService*>> services)
{
    quint32 size;
    HRESULT hr;
    hr = services->get_Size(&size);
    Q_ASSERT_SUCCEEDED(hr);
    for (quint32 i = 0; i < size; ++i) {
        ComPtr<IRfcommDeviceService> service;
        hr = services->GetAt(i, &service);
        Q_ASSERT_SUCCEEDED(hr);
        HString name;
        hr = service->get_ConnectionServiceName(name.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);
        const QString serviceName = QString::fromWCharArray(WindowsGetStringRawBuffer(name.Get(), nullptr));
        ComPtr<ABI::Windows::Networking::IHostName> host;
        hr = service->get_ConnectionHostName(host.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);
        HString hostName;
        hr = host->get_RawName(hostName.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);
        const QString qHostName = QString::fromWCharArray(WindowsGetStringRawBuffer(hostName.Get(),
                                                                                    nullptr));
        ComPtr<IRfcommServiceId> id;
        hr = service->get_ServiceId(&id);
        Q_ASSERT_SUCCEEDED(hr);
        GUID guid;
        hr = id->get_Uuid(&guid);
        const QBluetoothUuid uuid(guid);
        Q_ASSERT_SUCCEEDED(hr);

        QBluetoothServiceInfo info;
        info.setAttribute(0xBEEF, QVariant(qHostName));
        info.setAttribute(0xBEF0, QVariant(serviceName));
        info.setServiceName(serviceName);
        info.setServiceUuid(uuid);
        ComPtr<IAsyncOperation<IMapView<UINT32, IBuffer *> *>> op;
        hr = service->GetSdpRawAttributesAsync(op.GetAddressOf());
        if (FAILED(hr)) {
            emit errorOccured();
            qCDebug(QT_BT_WINDOWS) << "Check manifest capabilities";
            continue;
        }
        ComPtr<IMapView<UINT32, IBuffer *>> mapView;
        hr = QWinRTFunctions::await(op, mapView.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);
        // TODO timeout
        ComPtr<ValueIterable> iterable;
        ComPtr<ValueIterator> iterator;

        hr = mapView.As(&iterable);
        if (FAILED(hr))
            continue;

        boolean current = false;
        hr = iterable->First(&iterator);
        if (FAILED(hr))
            continue;
        hr = iterator->get_HasCurrent(&current);
        if (FAILED(hr))
            continue;

        while (SUCCEEDED(hr) && current) {
            ComPtr<ValueItem> item;
            hr = iterator->get_Current(&item);
            if (FAILED(hr))
                continue;

            UINT32 key;
            hr = item->get_Key(&key);
            if (FAILED(hr))
                continue;

            ComPtr<IBuffer> buffer;
            hr = item->get_Value(&buffer);
            Q_ASSERT_SUCCEEDED(hr);

            ComPtr<IDataReader> dataReader;
            ComPtr<IDataReaderStatics> dataReaderStatics;
            hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Storage_Streams_DataReader).Get(), &dataReaderStatics);
            Q_ASSERT_SUCCEEDED(hr);
            hr = dataReaderStatics->FromBuffer(buffer.Get(), dataReader.GetAddressOf());
            Q_ASSERT_SUCCEEDED(hr);
            BYTE type;
            hr = dataReader->ReadByte(&type);
            Q_ASSERT_SUCCEEDED(hr);
            if (type == TYPE_UINT8) {
                quint8 value;
                hr = dataReader->ReadByte(&value);
                Q_ASSERT_SUCCEEDED(hr);
                info.setAttribute(key, value);
                qCDebug(QT_BT_WINDOWS) << "UUID" << uuid << "KEY" << Qt::hex << key << "TYPE" << Qt::dec << type << "UINT8" << Qt::hex << value;
            } else if (type == TYPE_UINT16) {
                quint16 value;
                hr = dataReader->ReadUInt16(&value);
                Q_ASSERT_SUCCEEDED(hr);
                info.setAttribute(key, value);
                qCDebug(QT_BT_WINDOWS) << "UUID" << uuid << "KEY" << Qt::hex << key << "TYPE" << Qt::dec << type << "UINT16" << Qt::hex << value;
            } else if (type == TYPE_UINT32) {
                quint32 value;
                hr = dataReader->ReadUInt32(&value);
                Q_ASSERT_SUCCEEDED(hr);
                info.setAttribute(key, value);
                qCDebug(QT_BT_WINDOWS) << "UUID" << uuid << "KEY" << Qt::hex << key << "TYPE" << Qt::dec << type << "UINT32" << Qt::hex << value;
            } else if (type == TYPE_SHORT_UUID) {
                quint16 value;
                hr = dataReader->ReadUInt16(&value);
                Q_ASSERT_SUCCEEDED(hr);
                const QBluetoothUuid uuid(value);
                info.setAttribute(key, uuid);
                qCDebug(QT_BT_WINDOWS) << "UUID" << uuid << "KEY" << Qt::hex << key << "TYPE" << Qt::dec << type << "UUID" << Qt::hex << uuid;
            } else if (type == TYPE_LONG_UUID) {
                GUID value;
                hr = dataReader->ReadGuid(&value);
                Q_ASSERT_SUCCEEDED(hr);
                // The latter 8 bytes are in reverse order
                reverseArray(value.Data4, sizeof(value.Data4)/sizeof(value.Data4[0]));
                const QBluetoothUuid uuid(value);
                info.setAttribute(key, uuid);
                qCDebug(QT_BT_WINDOWS) << "UUID" << uuid << "KEY" << Qt::hex << key << "TYPE" << Qt::dec << type << "UUID" << Qt::hex << uuid;
            } else if (type == TYPE_STRING) {
                BYTE length;
                hr = dataReader->ReadByte(&length);
                Q_ASSERT_SUCCEEDED(hr);
                HString value;
                hr = dataReader->ReadString(length, value.GetAddressOf());
                Q_ASSERT_SUCCEEDED(hr);
                const QString str = QString::fromWCharArray(WindowsGetStringRawBuffer(value.Get(), nullptr));
                info.setAttribute(key, str);
                qCDebug(QT_BT_WINDOWS) << "UUID" << uuid << "KEY" << Qt::hex << key << "TYPE" << Qt::dec << type << "STRING" << str;
            } else if (type == TYPE_SEQUENCE) {
                bool ok;
                QBluetoothServiceInfo::Sequence sequence = readSequence(dataReader, &ok, nullptr);
                if (ok) {
                    info.setAttribute(key, sequence);
                    qCDebug(QT_BT_WINDOWS) << "UUID" << uuid << "KEY" << Qt::hex << key << "TYPE" << Qt::dec << type << "SEQUENCE" << sequence;
                } else {
                    qCDebug(QT_BT_WINDOWS) << "UUID" << uuid << "KEY" << Qt::hex << key << "TYPE" << Qt::dec << type << "SEQUENCE ERROR";
                }
            } else {
                qCDebug(QT_BT_WINDOWS) << "UUID" << uuid << "KEY" << Qt::hex << key << "TYPE" << Qt::dec << type;
            }
            hr = iterator->MoveNext(&current);
        }
        // Windows is only able to discover Rfcomm services but the according protocolDescriptor is
        // not always set in the raw attribute map. If we encounter a service like that we should
        // fill the protocol descriptor ourselves.
        if (info.protocolDescriptor(QBluetoothUuid::ProtocolUuid::Rfcomm).isEmpty()) {
            QBluetoothServiceInfo::Sequence protocolDescriptorList;
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
                     << QVariant::fromValue(0);
            protocolDescriptorList.append(QVariant::fromValue(protocol));
            info.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);
        }
        emit serviceFound(address, info);
    }
    emit scanFinished(address);
    deleteLater();
}

QBluetoothServiceInfo::Sequence QWinRTBluetoothServiceDiscoveryWorker::readSequence(ComPtr<IDataReader> dataReader, bool *ok, quint8 *bytesRead, int depth)
{
    if (ok)
        *ok = false;
    if (bytesRead)
        *bytesRead = 0;
    QBluetoothServiceInfo::Sequence result;
    if (!dataReader)
        return result;

    if (depth > QBluetoothServiceDiscoveryAgentPrivate::kMaxSdpRecursionDepth) {
        qCWarning(QT_BT_WINDOWS) << "SDP sequence recursion depth exceeded";
        return result;
    }

    quint8 remainingLength;
    HRESULT hr = dataReader->ReadByte(&remainingLength);
    Q_ASSERT_SUCCEEDED(hr);
    if (bytesRead)
        *bytesRead += 1;
    BYTE type;
    hr = dataReader->ReadByte(&type);
    remainingLength -= 1;
    if (bytesRead)
        *bytesRead += 1;
    Q_ASSERT_SUCCEEDED(hr);
    while (true) {
        switch (type) {
        case TYPE_UINT8: {
            quint8 value;
            hr = dataReader->ReadByte(&value);
            Q_ASSERT_SUCCEEDED(hr);
            result.append(QVariant::fromValue(value));
            remainingLength -= 1;
            if (bytesRead)
                *bytesRead += 1;
            break;
        }
        case TYPE_UINT16: {
            quint16 value;
            hr = dataReader->ReadUInt16(&value);
            Q_ASSERT_SUCCEEDED(hr);
            result.append(QVariant::fromValue(value));
            remainingLength -= 2;
            if (bytesRead)
                *bytesRead += 2;
            break;
        }
        case TYPE_UINT32: {
            quint32 value;
            hr = dataReader->ReadUInt32(&value);
            Q_ASSERT_SUCCEEDED(hr);
            result.append(QVariant::fromValue(value));
            remainingLength -= 4;
            if (bytesRead)
                *bytesRead += 4;
            break;
        }
        case TYPE_SHORT_UUID: {
            quint16 b;
            hr = dataReader->ReadUInt16(&b);
            Q_ASSERT_SUCCEEDED(hr);

            const QBluetoothUuid uuid(b);
            result.append(QVariant::fromValue(uuid));
            remainingLength -= 2;
            if (bytesRead)
                *bytesRead += 2;
            break;
        }
        case TYPE_LONG_UUID: {
            GUID b;
            hr = dataReader->ReadGuid(&b);
            Q_ASSERT_SUCCEEDED(hr);
            // The latter 8 bytes are in reverse order
            reverseArray(b.Data4, sizeof(b.Data4)/sizeof(b.Data4[0]));
            const QBluetoothUuid uuid(b);
            result.append(QVariant::fromValue(uuid));
            remainingLength -= sizeof(GUID);
            if (bytesRead)
                *bytesRead += sizeof(GUID);
            break;
        }
        case TYPE_STRING: {
            BYTE length;
            hr = dataReader->ReadByte(&length);
            Q_ASSERT_SUCCEEDED(hr);
            remainingLength -= 1;
            if (bytesRead)
                *bytesRead += 1;
            HString value;
            hr = dataReader->ReadString(length, value.GetAddressOf());
            Q_ASSERT_SUCCEEDED(hr);

            const QString str = QString::fromWCharArray(WindowsGetStringRawBuffer(value.Get(), nullptr));
            result.append(QVariant::fromValue(str));
            remainingLength -= length;
            if (bytesRead)
                *bytesRead += length;
            break;
        }
        case TYPE_SEQUENCE: {
            quint8 bytesR;
            const QBluetoothServiceInfo::Sequence sequence = readSequence(dataReader, ok, &bytesR, depth + 1);
            if (*ok)
                result.append(QVariant::fromValue(sequence));
            else
                return result;
            remainingLength -= bytesR;
            if (bytesRead)
                *bytesRead += bytesR;
            break;
        }
        default:
            qCDebug(QT_BT_WINDOWS) << "SEQUENCE ERROR" << type;
            result.clear();
            return result;
        }
        if (remainingLength == 0)
            break;

        hr = dataReader->ReadByte(&type);
        Q_ASSERT_SUCCEEDED(hr);
        remainingLength -= 1;
        if (bytesRead)
            *bytesRead += 1;
    }

    if (ok)
        *ok = true;
    return result;
}

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
    QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &deviceAdapter)
    : error(QBluetoothServiceDiscoveryAgent::NoError),
      state(Inactive),
      mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery),
      singleDevice(false),
      q_ptr(qp)
{
    mainThreadCoInit(this);
    // TODO: use local adapter for discovery. Possible?
    Q_UNUSED(deviceAdapter);
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
    releaseWorker();
    mainThreadCoUninit(this);
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    if (worker)
        return;

    worker = new QWinRTBluetoothServiceDiscoveryWorker(address.toUInt64(), mode);

    connect(worker, &QWinRTBluetoothServiceDiscoveryWorker::serviceFound,
        this, &QBluetoothServiceDiscoveryAgentPrivate::processFoundService, Qt::QueuedConnection);
    connect(worker, &QWinRTBluetoothServiceDiscoveryWorker::scanFinished,
        this, &QBluetoothServiceDiscoveryAgentPrivate::onScanFinished, Qt::QueuedConnection);
    connect(worker, &QWinRTBluetoothServiceDiscoveryWorker::errorOccured,
        this, &QBluetoothServiceDiscoveryAgentPrivate::onError, Qt::QueuedConnection);
    worker->start();
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    releaseWorker();
    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->canceled();
}

void QBluetoothServiceDiscoveryAgentPrivate::processFoundService(quint64 deviceAddress, const QBluetoothServiceInfo &info)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);
    //apply uuidFilter
    if (!uuidFilter.isEmpty()) {
        bool serviceNameMatched = uuidFilter.contains(info.serviceUuid());
        bool serviceClassMatched = false;
        const QList<QBluetoothUuid> serviceClassUuids
                 = info.serviceClassUuids();
        for (const QBluetoothUuid &id : serviceClassUuids) {
            if (uuidFilter.contains(id)) {
                serviceClassMatched = true;
                break;
            }
        }

        if (!serviceNameMatched && !serviceClassMatched)
            return;
    }

    if (!info.isValid())
        return;

    QBluetoothServiceInfo returnInfo(info);
    bool deviceFound;
    for (const QBluetoothDeviceInfo &deviceInfo : std::as_const(discoveredDevices)) {
        if (deviceInfo.address().toUInt64() == deviceAddress) {
            deviceFound = true;
            returnInfo.setDevice(deviceInfo);
            break;
        }
    }
    Q_ASSERT(deviceFound);

    if (!isDuplicatedService(returnInfo)) {
        discoveredServices.append(returnInfo);
        qCDebug(QT_BT_WINDOWS) << "Discovered services" << discoveredDevices.at(0).address().toString()
                             << returnInfo.serviceName() << returnInfo.serviceUuid()
                             << ">>>" << returnInfo.serviceClassUuids();

        emit q->serviceDiscovered(returnInfo);
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::onScanFinished(quint64 deviceAddress)
{
    // The scan for a device's services has finished. Disconnect the
    // worker and call the baseclass function which starts the scan for
    // the next device if there are any unscanned devices left (or finishes
    // the scan if none left)
    releaseWorker();
    _q_serviceDiscoveryFinished();
}

void QBluetoothServiceDiscoveryAgentPrivate::onError()
{
    Q_Q(QBluetoothServiceDiscoveryAgent);
    discoveredDevices.clear();
    error = QBluetoothServiceDiscoveryAgent::InputOutputError;
    errorString = QStringLiteral("errorDescription");
    emit q->errorOccurred(error);
}

void QBluetoothServiceDiscoveryAgentPrivate::releaseWorker()
{
    if (!worker)
        return;

    disconnect(worker, &QWinRTBluetoothServiceDiscoveryWorker::serviceFound,
        this, &QBluetoothServiceDiscoveryAgentPrivate::processFoundService);
    disconnect(worker, &QWinRTBluetoothServiceDiscoveryWorker::scanFinished,
        this, &QBluetoothServiceDiscoveryAgentPrivate::onScanFinished);
    disconnect(worker, &QWinRTBluetoothServiceDiscoveryWorker::errorOccured,
        this, &QBluetoothServiceDiscoveryAgentPrivate::onError);
    // mWorker will delete itself as soon as it is done with its discovery
    worker = nullptr;
}

QT_END_NAMESPACE

#include <qbluetoothservicediscoveryagent_winrt.moc>
