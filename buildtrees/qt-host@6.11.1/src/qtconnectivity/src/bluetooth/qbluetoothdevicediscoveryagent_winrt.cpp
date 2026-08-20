// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent_p.h"
#include "qbluetoothaddress.h"
#include "qbluetoothuuid.h"

#include <QtBluetooth/private/qbluetoothdevicewatcher_winrt_p.h>
#include <QtBluetooth/private/qbluetoothutils_winrt_p.h>
#include <QtBluetooth/private/qtbluetoothglobal_p.h>

#include <QtCore/QLoggingCategory>
#include <QtCore/QMutex>
#include <QtCore/qendian.h>

#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.Advertisement.h>
#include <winrt/Windows.Devices.Bluetooth.Rfcomm.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::Advertisement;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace winrt::Windows::Devices::Bluetooth::Rfcomm;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage::Streams;

QT_BEGIN_NAMESPACE

QT_IMPL_METATYPE_EXTERN(ManufacturerData)
QT_IMPL_METATYPE_EXTERN(ServiceData)

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)

static QByteArray byteArrayFromBuffer(const IBuffer &buffer)
{
    const uint8_t *data = buffer.data();
    return QByteArray(reinterpret_cast<const char *>(data),
                      static_cast<qsizetype>(buffer.Length()));
}

static ManufacturerData extractManufacturerData(const BluetoothLEAdvertisement &ad)
{
    ManufacturerData ret;
    const auto data = ad.ManufacturerData();
    for (const auto &item : data) {
        const uint16_t id = item.CompanyId();
        const QByteArray bufferData = byteArrayFromBuffer(item.Data());
        if (ret.contains(id))
            qCWarning(QT_BT_WINDOWS) << "Company ID already present in manufacturer data.";
        ret.insert(id, bufferData);
    }
    return ret;
}

static ServiceData extractServiceData(const BluetoothLEAdvertisement &ad)
{
    static constexpr int serviceDataTypes[3] = { 0x16, 0x20, 0x21 };

    ServiceData ret;

    for (const auto &serviceDataType : serviceDataTypes) {
        const auto dataSections = ad.GetSectionsByType(serviceDataType);
        for (const auto &section : dataSections) {
            const unsigned char dataType = section.DataType();
            const QByteArray bufferData = byteArrayFromBuffer(section.Data());
            if (dataType == 0x16) {
                Q_ASSERT(bufferData.size() >= 2);
                ret.insert(QBluetoothUuid(qFromLittleEndian<quint16>(bufferData.constData())),
                           bufferData.right(bufferData.length() - 2));
            } else if (dataType == 0x20) {
                Q_ASSERT(bufferData.size() >= 4);
                ret.insert(QBluetoothUuid(qFromLittleEndian<quint32>(bufferData.constData())),
                           bufferData.right(bufferData.length() - 4));
            } else if (dataType == 0x21) {
                Q_ASSERT(bufferData.size() >= 16);
                ret.insert(QBluetoothUuid(qToBigEndian<QUuid::Id128Bytes>(
                                   qFromLittleEndian<QUuid::Id128Bytes>(bufferData.constData()))),
                           bufferData.right(bufferData.length() - 16));
            }
        }
    }

    return ret;
}

// Needed because there is no explicit conversion
static GUID fromWinRtGuid(const winrt::guid &guid)
{
    const GUID uuid {
        guid.Data1,
        guid.Data2,
        guid.Data3,
        { guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
          guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7] }
    };
    return uuid;
}

class AdvertisementWatcherWrapper : public QObject,
                                    public std::enable_shared_from_this<AdvertisementWatcherWrapper>
{
    Q_OBJECT
public:
    AdvertisementWatcherWrapper() {}
    ~AdvertisementWatcherWrapper()
    {
        stop();
    }
    void init()
    {
        m_watcher.AllowExtendedAdvertisements(true);
        m_watcher.ScanningMode(BluetoothLEScanningMode::Active);
    }
    void start() {
        subscribeToEvents();
        m_watcher.Start();
    }
    void stop()
    {
        if (canStop()) {
            unsubscribeFromEvents();
            m_watcher.Stop();
        }
    }

signals:
    // The signal will be emitted from a separate thread,
    // so we need to use Qt::QueuedConnection
    void advertisementDataReceived(quint64 address, qint16 rssi,
                                   const ManufacturerData &manufacturerData,
                                   const ServiceData &serviceData,
                                   const QList<QBluetoothUuid> &uuids);
private:
    void subscribeToEvents()
    {
        // The callbacks are triggered from separate threads. So we capture
        // thisPtr to make sure that the object is valid.
        auto thisPtr = shared_from_this();
        m_receivedToken = m_watcher.Received(
            [thisPtr](BluetoothLEAdvertisementWatcher,
                      BluetoothLEAdvertisementReceivedEventArgs args) {
                const uint64_t address = args.BluetoothAddress();
                const short rssi = args.RawSignalStrengthInDBm();
                const BluetoothLEAdvertisement ad = args.Advertisement();

                const ManufacturerData manufacturerData = extractManufacturerData(ad);
                const ServiceData serviceData = extractServiceData(ad);

                QList<QBluetoothUuid> serviceUuids;
                const auto guids = ad.ServiceUuids();
                for (const auto &guid : guids) {
                    const GUID uuid = fromWinRtGuid(guid);
                    serviceUuids.append(QBluetoothUuid(uuid));
                }

                emit thisPtr->advertisementDataReceived(address, rssi, manufacturerData,
                                                        serviceData, serviceUuids);
            });
    }
    void unsubscribeFromEvents()
    {
        m_watcher.Received(m_receivedToken);
    }
    bool canStop() const
    {
        const auto status = m_watcher.Status();
        return status == BluetoothLEAdvertisementWatcherStatus::Started
                || status == BluetoothLEAdvertisementWatcherStatus::Aborted;
    }

    BluetoothLEAdvertisementWatcher m_watcher;
    winrt::event_token m_receivedToken;
};

// Both constants are taken from Microsoft's docs:
// https://docs.microsoft.com/en-us/windows/uwp/devices-sensors/aep-service-class-ids
// Alternatively we could create separate watchers for paired and unpaired devices.
static const winrt::hstring ClassicDeviceSelector =
        L"System.Devices.Aep.ProtocolId:=\"{e0cbf06c-cd8b-4647-bb8a-263b43f0f974}\"";
// Do not use it for now, so comment out. Do not delete in case we want to reuse it.
//static const winrt::hstring LowEnergyDeviceSelector =
//        L"System.Devices.Aep.ProtocolId:=\"{bb7bb05e-5972-42b5-94fc-76eaa7084d49}\"";

class QWinRTBluetoothDeviceDiscoveryWorker : public QObject,
        public std::enable_shared_from_this<QWinRTBluetoothDeviceDiscoveryWorker>
{
    Q_OBJECT
public:
    QWinRTBluetoothDeviceDiscoveryWorker(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods,
                                         int interval);
    ~QWinRTBluetoothDeviceDiscoveryWorker();
    void start();
    void stop();

private:
    void startDeviceDiscovery(QBluetoothDeviceDiscoveryAgent::DiscoveryMethod mode);

    std::shared_ptr<QBluetoothDeviceWatcherWinRT> createDeviceWatcher(winrt::hstring selector,
                                                                      int watcherId);
    void generateError(QBluetoothDeviceDiscoveryAgent::Error error, const char *msg = nullptr);
    void invokeDeviceFoundWithDebug(const QBluetoothDeviceInfo &info);
    void finishDiscovery();
    bool isFinished() const;

    // Bluetooth Classic handlers
    void getClassicDeviceFromId(const winrt::hstring &id);
    void handleClassicDevice(const BluetoothDevice &device);
    void handleRfcommServices(const RfcommDeviceServicesResult &servicesResult,
                              uint64_t address, const QString &name, uint32_t classOfDeviceInt,
                              bool isCached);

    // Bluetooth Low Energy handlers
    void getLowEnergyDeviceFromId(const winrt::hstring &id);
    void handleLowEnergyDevice(const BluetoothLEDevice &device);
    void handleGattServices(const GattDeviceServicesResult &servicesResult,
                            QBluetoothDeviceInfo &info);

    // Bluetooth Low Energy Advertising handlers
    std::shared_ptr<AdvertisementWatcherWrapper> createAdvertisementWatcher();

    // invokable methods for handling finish conditions
    Q_INVOKABLE void decrementPendingDevicesCountAndCheckFinished(
            std::shared_ptr<QWinRTBluetoothDeviceDiscoveryWorker> worker);

Q_SIGNALS:
    void deviceFound(const QBluetoothDeviceInfo &info);
    void deviceDataChanged(const QBluetoothAddress &address, QBluetoothDeviceInfo::Fields,
                           qint16 rssi, ManufacturerData manufacturerData, ServiceData serviceData);
    void errorOccured(QBluetoothDeviceDiscoveryAgent::Error error);
    void scanFinished();

private slots:
    void onBluetoothDeviceFound(winrt::hstring deviceId, int watcherId);
    void onDeviceEnumerationCompleted(int watcherId);

    void onAdvertisementDataReceived(quint64 address, qint16 rssi,
                                     const ManufacturerData &manufacturerData,
                                     const ServiceData &serviceData,
                                     const QList<QBluetoothUuid> &uuids);

    void stopAdvertisementWatcher();

private:
    struct LEAdvertisingInfo {
        QList<QBluetoothUuid> services;
        ManufacturerData manufacturerData;
        ServiceData serviceData;
        qint16 rssi = 0;
    };

    quint8 requestedModes = 0;
    QMutex m_leDevicesMutex;
    QMap<quint64, LEAdvertisingInfo> m_foundLEDevicesMap;
    int m_pendingDevices = 0;

    static constexpr int ClassicWatcherId = 1;
    static constexpr int LowEnergyWatcherId = 2;

    std::shared_ptr<QBluetoothDeviceWatcherWinRT> m_classicWatcher;
    std::shared_ptr<QBluetoothDeviceWatcherWinRT> m_lowEnergyWatcher;
    std::shared_ptr<AdvertisementWatcherWrapper> m_advertisementWatcher;
    bool m_classicScanStarted = false;
    bool m_lowEnergyScanStarted = false;
    QTimer *m_leScanTimer = nullptr;
};

static void invokeDecrementPendingDevicesCountAndCheckFinished(
        std::shared_ptr<QWinRTBluetoothDeviceDiscoveryWorker> worker)
{
    QMetaObject::invokeMethod(worker.get(), "decrementPendingDevicesCountAndCheckFinished",
                              Qt::QueuedConnection,
                              Q_ARG(std::shared_ptr<QWinRTBluetoothDeviceDiscoveryWorker>,
                                    worker));
}

QWinRTBluetoothDeviceDiscoveryWorker::QWinRTBluetoothDeviceDiscoveryWorker(
        QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods, int interval)
    : requestedModes(methods)
{
    qRegisterMetaType<QBluetoothDeviceInfo>();
    qRegisterMetaType<QBluetoothDeviceInfo::Fields>();
    qRegisterMetaType<ManufacturerData>();
    qRegisterMetaType<std::shared_ptr<QWinRTBluetoothDeviceDiscoveryWorker>>();

    m_classicWatcher = createDeviceWatcher(ClassicDeviceSelector, ClassicWatcherId);
    // For LE scan use DeviceWatcher to handle only paired devices.
    // Non-paired devices will be found using BluetoothLEAdvertisementWatcher.
    const auto leSelector = BluetoothLEDevice::GetDeviceSelectorFromPairingState(true);
    m_lowEnergyWatcher = createDeviceWatcher(leSelector, LowEnergyWatcherId);
    m_advertisementWatcher = createAdvertisementWatcher();

    // Docs claim that a negative interval means that the backend handles it on its own
    if (interval < 0)
        interval = 40000;

    if (interval != 0) {
        m_leScanTimer = new QTimer(this);
        m_leScanTimer->setSingleShot(true);
        m_leScanTimer->setInterval(interval);
        connect(m_leScanTimer, &QTimer::timeout, this,
                &QWinRTBluetoothDeviceDiscoveryWorker::stopAdvertisementWatcher);
    }
}

QWinRTBluetoothDeviceDiscoveryWorker::~QWinRTBluetoothDeviceDiscoveryWorker()
{
    stop();
}

void QWinRTBluetoothDeviceDiscoveryWorker::start()
{
    if (requestedModes & QBluetoothDeviceDiscoveryAgent::ClassicMethod) {
        if (m_classicWatcher && m_classicWatcher->init()) {
            m_classicWatcher->start();
            m_classicScanStarted = true;
        } else {
            generateError(QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                          "Could not start classic device watcher");
        }
    }
    if (requestedModes & QBluetoothDeviceDiscoveryAgent::LowEnergyMethod) {
        if (m_lowEnergyWatcher && m_lowEnergyWatcher->init()) {
            m_lowEnergyWatcher->start();
            m_lowEnergyScanStarted = true;
        } else {
            generateError(QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                          "Could not start low energy device watcher");
        }
        if (m_advertisementWatcher) {
            m_advertisementWatcher->init();
            m_advertisementWatcher->start();
            if (m_leScanTimer)
                m_leScanTimer->start();
        } else {
            generateError(QBluetoothDeviceDiscoveryAgent::Error::UnknownError,
                          "Could not start low energy advertisement watcher");
        }
    }

    qCDebug(QT_BT_WINDOWS) << "Worker started";
}

void QWinRTBluetoothDeviceDiscoveryWorker::stop()
{
    if (m_leScanTimer && m_leScanTimer->isActive())
        m_leScanTimer->stop();
    m_classicWatcher->stop();
    m_lowEnergyWatcher->stop();
    m_advertisementWatcher->stop();
}

void QWinRTBluetoothDeviceDiscoveryWorker::finishDiscovery()
{
    stop();
    emit scanFinished();
}

bool QWinRTBluetoothDeviceDiscoveryWorker::isFinished() const
{
    // If the interval is set to 0, we do not start a timer, and that means
    // that we need to wait for the user to explicitly call stop()
    return (m_pendingDevices == 0) && !m_lowEnergyScanStarted && !m_classicScanStarted
            && (m_leScanTimer && !m_leScanTimer->isActive());
}

void QWinRTBluetoothDeviceDiscoveryWorker::onBluetoothDeviceFound(winrt::hstring deviceId, int watcherId)
{
    if (watcherId == ClassicWatcherId)
        getClassicDeviceFromId(deviceId);
    else if (watcherId == LowEnergyWatcherId)
        getLowEnergyDeviceFromId(deviceId);
}

void QWinRTBluetoothDeviceDiscoveryWorker::onDeviceEnumerationCompleted(int watcherId)
{
    qCDebug(QT_BT_WINDOWS) << (watcherId == ClassicWatcherId ? "BT" : "BTLE")
                           << "enumeration completed";
    if (watcherId == ClassicWatcherId) {
        m_classicWatcher->stop();
        m_classicScanStarted = false;
    } else if (watcherId == LowEnergyWatcherId) {
        m_lowEnergyWatcher->stop();
        m_lowEnergyScanStarted = false;
    }
    if (isFinished())
        finishDiscovery();
}

// this function executes in main worker thread
void QWinRTBluetoothDeviceDiscoveryWorker::onAdvertisementDataReceived(
        quint64 address, qint16 rssi, const ManufacturerData &manufacturerData,
        const ServiceData &serviceData, const QList<QBluetoothUuid> &uuids)
{
    // Merge newly found services with list of currently found ones
    bool needDiscoverServices = false;
    {
        QMutexLocker locker(&m_leDevicesMutex);
        if (m_foundLEDevicesMap.contains(address)) {
            QBluetoothDeviceInfo::Fields changedFields = QBluetoothDeviceInfo::Field::None;
            const LEAdvertisingInfo adInfo = m_foundLEDevicesMap.value(address);
            QList<QBluetoothUuid> foundServices = adInfo.services;
            if (adInfo.rssi != rssi) {
                m_foundLEDevicesMap[address].rssi = rssi;
                changedFields.setFlag(QBluetoothDeviceInfo::Field::RSSI);
            }
            if (adInfo.manufacturerData != manufacturerData) {
                m_foundLEDevicesMap[address].manufacturerData.insert(manufacturerData);
                if (adInfo.manufacturerData != m_foundLEDevicesMap[address].manufacturerData)
                    changedFields.setFlag(QBluetoothDeviceInfo::Field::ManufacturerData);
            }
            if (adInfo.serviceData != serviceData) {
                m_foundLEDevicesMap[address].serviceData.insert(serviceData);
                if (adInfo.serviceData != m_foundLEDevicesMap[address].serviceData)
                    changedFields.setFlag((QBluetoothDeviceInfo::Field::ServiceData));
            }
            for (const QBluetoothUuid &uuid : std::as_const(uuids)) {
                if (!foundServices.contains(uuid)) {
                    foundServices.append(uuid);
                    needDiscoverServices = true;
                }
            }
            if (!needDiscoverServices) {
                if (!changedFields.testFlag(QBluetoothDeviceInfo::Field::None)) {
                    QMetaObject::invokeMethod(this, "deviceDataChanged", Qt::AutoConnection,
                                              Q_ARG(QBluetoothAddress, QBluetoothAddress(address)),
                                              Q_ARG(QBluetoothDeviceInfo::Fields, changedFields),
                                              Q_ARG(qint16, rssi),
                                              Q_ARG(ManufacturerData, manufacturerData),
                                              Q_ARG(ServiceData, serviceData));
                }
            }
            m_foundLEDevicesMap[address].services = foundServices;
        } else {
            needDiscoverServices = true;
            LEAdvertisingInfo info;
            info.services = std::move(uuids);
            info.manufacturerData = std::move(manufacturerData);
            info.serviceData = std::move(serviceData);
            info.rssi = rssi;
            m_foundLEDevicesMap.insert(address, info);
        }
    }
    if (needDiscoverServices) {
        ++m_pendingDevices; // as if we discovered a new LE device
        auto thisPtr = shared_from_this();
        auto asyncOp = BluetoothLEDevice::FromBluetoothAddressAsync(address);
        asyncOp.Completed([thisPtr, address](auto &&op, AsyncStatus status) {
            if (thisPtr) {
                if (status == AsyncStatus::Completed) {
                    BluetoothLEDevice device = op.GetResults();
                    if (device) {
                        thisPtr->handleLowEnergyDevice(device);
                        return;
                    }
                }
                // status != Completed or failed to extract result
                qCDebug(QT_BT_WINDOWS) << "Failed to get LE device from address"
                                       << QBluetoothAddress(address);
                invokeDecrementPendingDevicesCountAndCheckFinished(thisPtr);
            }
        });
    }
}

void QWinRTBluetoothDeviceDiscoveryWorker::stopAdvertisementWatcher()
{
    m_advertisementWatcher->stop();
    if (isFinished())
        finishDiscovery();
}

std::shared_ptr<QBluetoothDeviceWatcherWinRT>
QWinRTBluetoothDeviceDiscoveryWorker::createDeviceWatcher(winrt::hstring selector, int watcherId)
{
    auto watcher = std::make_shared<QBluetoothDeviceWatcherWinRT>(
                watcherId, selector, DeviceInformationKind::AssociationEndpoint);
    if (watcher) {
        connect(watcher.get(), &QBluetoothDeviceWatcherWinRT::deviceAdded,
                this, &QWinRTBluetoothDeviceDiscoveryWorker::onBluetoothDeviceFound,
                Qt::QueuedConnection);
        connect(watcher.get(), &QBluetoothDeviceWatcherWinRT::enumerationCompleted,
                this, &QWinRTBluetoothDeviceDiscoveryWorker::onDeviceEnumerationCompleted,
                Qt::QueuedConnection);
    }
    return watcher;
}

void QWinRTBluetoothDeviceDiscoveryWorker::generateError(
        QBluetoothDeviceDiscoveryAgent::Error error, const char *msg)
{
    emit errorOccured(error);
    qCWarning(QT_BT_WINDOWS) << msg;
}

void QWinRTBluetoothDeviceDiscoveryWorker::invokeDeviceFoundWithDebug(const QBluetoothDeviceInfo &info)
{
    qCDebug(QT_BT_WINDOWS) << "Discovered BTLE device: " << info.address() << info.name()
                           << "Num UUIDs" << info.serviceUuids().size() << "RSSI:" << info.rssi()
                           << "Num manufacturer data" << info.manufacturerData().size()
                           << "Num service data" << info.serviceData().size();

    QMetaObject::invokeMethod(this, "deviceFound", Qt::AutoConnection,
                              Q_ARG(QBluetoothDeviceInfo, info));
}

// this function executes in main worker thread
void QWinRTBluetoothDeviceDiscoveryWorker::getClassicDeviceFromId(const winrt::hstring &id)
{
    ++m_pendingDevices;
    auto thisPtr = shared_from_this();
    auto asyncOp = BluetoothDevice::FromIdAsync(id);
    asyncOp.Completed([thisPtr](auto &&op, AsyncStatus status) {
        if (thisPtr) {
            if (status == AsyncStatus::Completed) {
                BluetoothDevice device = op.GetResults();
                if (device) {
                    thisPtr->handleClassicDevice(device);
                    return;
                }
            }
            // status != Completed or failed to extract result
            qCDebug(QT_BT_WINDOWS) << "Failed to get Classic device from id";
            invokeDecrementPendingDevicesCountAndCheckFinished(thisPtr);
        }
    });
}

// this is a callback - executes in a new thread
void QWinRTBluetoothDeviceDiscoveryWorker::handleClassicDevice(const BluetoothDevice &device)
{
    const uint64_t address = device.BluetoothAddress();
    const std::wstring name { device.Name() }; // via operator std::wstring_view()
    const QString btName = QString::fromStdWString(name);
    const uint32_t deviceClass = device.ClassOfDevice().RawValue();

    // Use IsPaired() to determine if the device is cached or not.
    // That is because Windows will always report paired devices, even if
    // they are not physically available. This is a sub-optimal approach,
    // but looks like we cannot do better...
    const bool isCached = device.DeviceInformation().Pairing().IsPaired();

    auto thisPtr = shared_from_this();
    auto asyncOp = device.GetRfcommServicesAsync();
    asyncOp.Completed([thisPtr, address, btName, deviceClass, isCached]
                      (auto &&op, AsyncStatus status) {
        if (thisPtr) {
            if (status == AsyncStatus::Completed) {
                auto servicesResult = op.GetResults();
                if (servicesResult) {
                    thisPtr->handleRfcommServices(servicesResult, address, btName,
                                                  deviceClass, isCached);
                    return;
                }
            }
            // Failed to get services
            qCDebug(QT_BT_WINDOWS) << "Failed to get RFCOMM services for device" << btName;
            invokeDecrementPendingDevicesCountAndCheckFinished(thisPtr);
        }
    });
}

// this is a callback - executes in a new thread
void QWinRTBluetoothDeviceDiscoveryWorker::handleRfcommServices(
        const RfcommDeviceServicesResult &servicesResult, uint64_t address,
        const QString &name, uint32_t classOfDeviceInt, bool isCached)
{
    // need to perform the check even if some of the operations fails
    auto shared = shared_from_this();
    auto guard = qScopeGuard([shared]() {
        invokeDecrementPendingDevicesCountAndCheckFinished(shared);
    });
    Q_UNUSED(guard); // to suppress warning

    const auto error = servicesResult.Error();
    if (error != BluetoothError::Success) {
        qCWarning(QT_BT_WINDOWS) << "Obtain device services completed with BluetoothError"
                                 << static_cast<int>(error);
        return;
    }

    const auto services = servicesResult.Services();
    QList<QBluetoothUuid> uuids;
    for (const auto &service : services) {
        const auto serviceId = service.ServiceId();
        const GUID uuid = fromWinRtGuid(serviceId.Uuid());
        uuids.append(QBluetoothUuid(uuid));
    }

    const QBluetoothAddress btAddress(address);

    qCDebug(QT_BT_WINDOWS) << "Discovered BT device: " << btAddress << name
                           << "Num UUIDs" << uuids.size();

    QBluetoothDeviceInfo info(btAddress, name, classOfDeviceInt);
    info.setCoreConfigurations(QBluetoothDeviceInfo::BaseRateCoreConfiguration);
    info.setServiceUuids(uuids);
    info.setCached(isCached);

    QMetaObject::invokeMethod(this, "deviceFound", Qt::AutoConnection,
                              Q_ARG(QBluetoothDeviceInfo, info));
}

void QWinRTBluetoothDeviceDiscoveryWorker::decrementPendingDevicesCountAndCheckFinished(
        std::shared_ptr<QWinRTBluetoothDeviceDiscoveryWorker> worker)
{
    --m_pendingDevices;
    if (isFinished())
        finishDiscovery();
    // Worker is passed here simply to make sure that the object is still alive
    // when we call this method via QObject::invoke().
    Q_UNUSED(worker)
}

// this function executes in main worker thread
void QWinRTBluetoothDeviceDiscoveryWorker::getLowEnergyDeviceFromId(const winrt::hstring &id)
{
    ++m_pendingDevices;
    auto asyncOp = BluetoothLEDevice::FromIdAsync(id);
    auto thisPtr = shared_from_this();
    asyncOp.Completed([thisPtr](auto &&op, AsyncStatus status) {
        if (thisPtr) {
            if (status == AsyncStatus::Completed) {
                BluetoothLEDevice device = op.GetResults();
                if (device) {
                    thisPtr->handleLowEnergyDevice(device);
                    return;
                }
            }
            // status != Completed or failed to extract result
            qCDebug(QT_BT_WINDOWS) << "Failed to get LE device from id";
            invokeDecrementPendingDevicesCountAndCheckFinished(thisPtr);
        }
    });
}

// this is a callback - executes in a new thread
void QWinRTBluetoothDeviceDiscoveryWorker::handleLowEnergyDevice(const BluetoothLEDevice &device)
{
    const uint64_t address = device.BluetoothAddress();
    const std::wstring name { device.Name() }; // via operator std::wstring_view()
    const QString btName = QString::fromStdWString(name);
    const bool isPaired = device.DeviceInformation().Pairing().IsPaired();

    m_leDevicesMutex.lock();
    const LEAdvertisingInfo adInfo = m_foundLEDevicesMap.value(address);
    m_leDevicesMutex.unlock();
    const ManufacturerData manufacturerData = adInfo.manufacturerData;
    const ServiceData serviceData = adInfo.serviceData;
    const qint16 rssi = adInfo.rssi;

    QBluetoothDeviceInfo info(QBluetoothAddress(address), btName, 0);
    info.setCoreConfigurations(QBluetoothDeviceInfo::LowEnergyCoreConfiguration);
    info.setRssi(rssi);
    for (quint16 key : manufacturerData.keys())
        info.setManufacturerData(key, manufacturerData.value(key));
    for (QBluetoothUuid key : serviceData.keys())
        info.setServiceData(key, serviceData.value(key));

    // Now we need to figure out if the device is cached or not.
    // Theoretically, we could use the "System.Devices.Aep.IsPresent"
    // device property. However, in practice it's not reported, so
    // let's try to do our best and estimate the value.
    // If the device is paired, then Windows will always report it,
    // no matter if it's actually present or not.
    // So, treat paired devices as cached at first. If the device is
    // actually available, we'll get an update (at least with a new
    // rssi), and update the cache state at that point.
    info.setCached(isPaired);

    // Use the services obtained from the advertisement data if the device is not paired
    if (!isPaired) {
        info.setServiceUuids(adInfo.services);
        invokeDecrementPendingDevicesCountAndCheckFinished(shared_from_this());
        invokeDeviceFoundWithDebug(info);
    } else {
        auto asyncOp = device.GetGattServicesAsync();
        auto thisPtr = shared_from_this();
        asyncOp.Completed([thisPtr, info](auto &&op, AsyncStatus status) mutable {
            if (status == AsyncStatus::Completed) {
                auto servicesResult = op.GetResults();
                if (servicesResult) {
                    thisPtr->handleGattServices(servicesResult, info);
                    return;
                }
            }
            // Failed to get services
            qCDebug(QT_BT_WINDOWS) << "Failed to get GATT services for device" << info.name();
            invokeDecrementPendingDevicesCountAndCheckFinished(thisPtr);
        });
    }
}

// this is a callback - executes in a new thread
void QWinRTBluetoothDeviceDiscoveryWorker::handleGattServices(
        const GattDeviceServicesResult &servicesResult, QBluetoothDeviceInfo &info)
{
    // need to perform the check even if some of the operations fails
    auto shared = shared_from_this();
    auto guard = qScopeGuard([shared]() {
        invokeDecrementPendingDevicesCountAndCheckFinished(shared);
    });
    Q_UNUSED(guard); // to suppress warning

    const auto status = servicesResult.Status();
    if (status == GattCommunicationStatus::Success) {
        const auto services = servicesResult.Services();
        QList<QBluetoothUuid> uuids;
        for (const auto &service : services) {
            const GUID uuid = fromWinRtGuid(service.Uuid());
            uuids.append(QBluetoothUuid(uuid));
        }
        info.setServiceUuids(uuids);
    } else {
        qCWarning(QT_BT_WINDOWS) << "Obtaining LE services finished with status"
                                 << static_cast<int>(status);
    }
    invokeDeviceFoundWithDebug(info);
}

std::shared_ptr<AdvertisementWatcherWrapper>
QWinRTBluetoothDeviceDiscoveryWorker::createAdvertisementWatcher()
{
    auto watcher = std::make_shared<AdvertisementWatcherWrapper>();
    if (watcher) {
        connect(watcher.get(), &AdvertisementWatcherWrapper::advertisementDataReceived,
                this, &QWinRTBluetoothDeviceDiscoveryWorker::onAdvertisementDataReceived,
                Qt::QueuedConnection);
    }
    return watcher;
}

QBluetoothDeviceDiscoveryAgentPrivate::QBluetoothDeviceDiscoveryAgentPrivate(
        const QBluetoothAddress &deviceAdapter, QBluetoothDeviceDiscoveryAgent *parent)
    : q_ptr(parent), adapterAddress(deviceAdapter)
{
    mainThreadCoInit(this);
}

QBluetoothDeviceDiscoveryAgentPrivate::~QBluetoothDeviceDiscoveryAgentPrivate()
{
    disconnectAndClearWorker();
    mainThreadCoUninit(this);
}

bool QBluetoothDeviceDiscoveryAgentPrivate::isActive() const
{
    return worker != nullptr;
}

QBluetoothDeviceDiscoveryAgent::DiscoveryMethods QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods()
{
    return (ClassicMethod | LowEnergyMethod);
}

void QBluetoothDeviceDiscoveryAgentPrivate::start(QBluetoothDeviceDiscoveryAgent::DiscoveryMethods methods)
{
    QBluetoothLocalDevice adapter(adapterAddress);
    if (!adapter.isValid()) {
        qCWarning(QT_BT_WINDOWS) << "Cannot find Bluetooth adapter for device search";
        lastError = QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Cannot find valid Bluetooth adapter.");
        emit q_ptr->errorOccurred(lastError);
        return;
    } else if (adapter.hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        qCWarning(QT_BT_WINDOWS) << "Bluetooth adapter powered off";
        lastError = QBluetoothDeviceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothDeviceDiscoveryAgent::tr("Bluetooth adapter powered off.");
        emit q_ptr->errorOccurred(lastError);
        return;
    }

    if (worker)
        return;

    worker = std::make_shared<QWinRTBluetoothDeviceDiscoveryWorker>(methods,
                                                                    lowEnergySearchTimeout);
    lastError = QBluetoothDeviceDiscoveryAgent::NoError;
    errorString.clear();
    discoveredDevices.clear();
    connect(worker.get(), &QWinRTBluetoothDeviceDiscoveryWorker::deviceFound,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::registerDevice);
    connect(worker.get(), &QWinRTBluetoothDeviceDiscoveryWorker::deviceDataChanged,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::updateDeviceData);
    connect(worker.get(), &QWinRTBluetoothDeviceDiscoveryWorker::errorOccured,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::onErrorOccured);
    connect(worker.get(), &QWinRTBluetoothDeviceDiscoveryWorker::scanFinished,
            this, &QBluetoothDeviceDiscoveryAgentPrivate::onScanFinished);
    worker->start();
}

void QBluetoothDeviceDiscoveryAgentPrivate::stop()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    if (worker) {
        worker->stop();
        disconnectAndClearWorker();
        emit q->canceled();
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::registerDevice(const QBluetoothDeviceInfo &info)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);

    for (QList<QBluetoothDeviceInfo>::iterator iter = discoveredDevices.begin();
        iter != discoveredDevices.end(); ++iter) {
        if (iter->address() == info.address()) {
            qCDebug(QT_BT_WINDOWS) << "Updating device" << iter->name() << iter->address();
            // merge service uuids
            QList<QBluetoothUuid> uuids = iter->serviceUuids();
            uuids.append(info.serviceUuids());
            const QSet<QBluetoothUuid> uuidSet(uuids.begin(), uuids.end());
            if (iter->serviceUuids().size() != uuidSet.size())
                iter->setServiceUuids(uuidSet.values().toVector());
            if (iter->coreConfigurations() != info.coreConfigurations())
                iter->setCoreConfigurations(QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration);
            return;
        }
    }

    discoveredDevices << info;
    emit q->deviceDiscovered(info);
}

void QBluetoothDeviceDiscoveryAgentPrivate::updateDeviceData(const QBluetoothAddress &address,
                                                             QBluetoothDeviceInfo::Fields fields,
                                                             qint16 rssi,
                                                             ManufacturerData manufacturerData,
                                                             ServiceData serviceData)
{
    if (fields.testFlag(QBluetoothDeviceInfo::Field::None))
        return;

    Q_Q(QBluetoothDeviceDiscoveryAgent);
    for (QList<QBluetoothDeviceInfo>::iterator iter = discoveredDevices.begin();
        iter != discoveredDevices.end(); ++iter) {
        if (iter->address() == address) {
            qCDebug(QT_BT_WINDOWS) << "Updating data for device" << iter->name() << iter->address();
            if (fields.testFlag(QBluetoothDeviceInfo::Field::RSSI))
                iter->setRssi(rssi);
            if (fields.testFlag(QBluetoothDeviceInfo::Field::ManufacturerData))
                for (quint16 key : manufacturerData.keys())
                    iter->setManufacturerData(key, manufacturerData.value(key));
            if (fields.testFlag(QBluetoothDeviceInfo::Field::ServiceData))
                for (QBluetoothUuid key : serviceData.keys())
                    iter->setServiceData(key, serviceData.value(key));
            // We got some data, so the device is definitely available now.
            // Update the cached state.
            iter->setCached(false);
            emit q->deviceUpdated(*iter, fields);
            return;
        }
    }
}

void QBluetoothDeviceDiscoveryAgentPrivate::onErrorOccured(QBluetoothDeviceDiscoveryAgent::Error e)
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    lastError = e;
    emit q->errorOccurred(e);
}

void QBluetoothDeviceDiscoveryAgentPrivate::onScanFinished()
{
    Q_Q(QBluetoothDeviceDiscoveryAgent);
    disconnectAndClearWorker();
    emit q->finished();
}

void QBluetoothDeviceDiscoveryAgentPrivate::disconnectAndClearWorker()
{
    if (!worker)
        return;

    disconnect(worker.get(), &QWinRTBluetoothDeviceDiscoveryWorker::scanFinished,
               this, &QBluetoothDeviceDiscoveryAgentPrivate::onScanFinished);
    disconnect(worker.get(), &QWinRTBluetoothDeviceDiscoveryWorker::deviceFound,
               this, &QBluetoothDeviceDiscoveryAgentPrivate::registerDevice);
    disconnect(worker.get(), &QWinRTBluetoothDeviceDiscoveryWorker::deviceDataChanged,
               this, &QBluetoothDeviceDiscoveryAgentPrivate::updateDeviceData);
    disconnect(worker.get(), &QWinRTBluetoothDeviceDiscoveryWorker::errorOccured,
               this, &QBluetoothDeviceDiscoveryAgentPrivate::onErrorOccured);

    worker = nullptr;
}

QT_END_NAMESPACE

#include <qbluetoothdevicediscoveryagent_winrt.moc>
