// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergycontroller_winrt_p.h"
#include "qbluetoothutils_winrt_p.h"

#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyDescriptorData>
#include <QtBluetooth/private/qbluetoothutils_winrt_p.h>
#include <QtBluetooth/QLowEnergyService>

#include <QtCore/QtEndian>
#include <QtCore/QLoggingCategory>
#include <QtCore/private/qfunctions_winrt_p.h>
#include <QtCore/QDeadlineTimer>
#include <QtCore/qpointer.h>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QDeadlineTimer>

#include <functional>
#include <type_traits>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.Devices.Enumeration.h>
#include <winrt/Windows.Devices.Bluetooth.h>
#include <winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h>
#include <winrt/Windows.Storage.Streams.h>

#include <windows.devices.bluetooth.h>
#include <windows.devices.bluetooth.genericattributeprofile.h>

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Devices;
using namespace winrt::Windows::Devices::Bluetooth;
using namespace winrt::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace winrt::Windows::Devices::Enumeration;
using namespace winrt::Windows::Storage::Streams;

namespace abi
{
    using BluetoothLEDevice = ABI::Windows::Devices::Bluetooth::IBluetoothLEDevice;
    using GattSession = ABI::Windows::Devices::Bluetooth::GenericAttributeProfile::IGattSession;
}

template<typename E,
    std::enable_if_t<std::is_enum_v<E>, int> = 0>
inline constexpr std::underlying_type_t<E> bitwise_and(E x, E y)
{
    return static_cast<std::underlying_type_t<E>>(x) & static_cast<std::underlying_type_t<E>>(y);
}

template<typename T, typename E,
    std::enable_if_t<std::conjunction_v<std::is_integral<T>, std::is_enum<E>>, int> = 0>
inline constexpr T bitwise_and(T x, E y)
{
    return x & static_cast<std::underlying_type_t<E>>(y);
}

template<typename E,
    std::enable_if_t<std::is_enum_v<E>, int> = 0>
inline constexpr std::underlying_type_t<E> bitwise_or(E x, E y)
{
    return static_cast<std::underlying_type_t<E>>(x) | static_cast<std::underlying_type_t<E>>(y);
}

template<typename T, typename E,
    std::enable_if_t<std::conjunction_v<std::is_integral<T>, std::is_enum<E>>, int> = 0>
inline constexpr T bitwise_or(T x, E y)
{
    return x | static_cast<std::underlying_type_t<E>>(y);
}

template<typename T, typename E,
    std::enable_if_t<std::conjunction_v<std::is_integral<T>, std::is_enum<E>>, int> = 0>
inline constexpr T &bitwise_or_equal(T &a, const E &b)
{
    a |= static_cast<std::underlying_type_t<E>>(b);
    return a;
}

#define ENUM_BITWISE_OPS(E) \
inline constexpr std::underlying_type_t<E> operator&(E x, E y) { return bitwise_and<E>(x, y); } \
\
template<typename T> \
inline constexpr T operator&(T x, E y) { return bitwise_and<T, E>(x, y); } \
\
template<typename T> \
inline constexpr T operator&(E x, T y) { return bitwise_and<T, E>(y, x); } \
\
inline constexpr std::underlying_type_t<E> operator|(E x, E y) { return bitwise_or<E>(x, y); } \
\
template<typename T> \
inline constexpr T operator|(T x, E y) { return bitwise_or<T, E>(x, y); } \
\
template<typename T> \
inline constexpr T operator|(E x, T y) { return bitwise_or<T, E>(y, x); } \
\
template<typename T> \
inline constexpr T &operator |=(T &a, const E &b) { return bitwise_or_equal(a, b); }

QT_BEGIN_NAMESPACE

ENUM_BITWISE_OPS(GattClientCharacteristicConfigurationDescriptorValue)

typedef GattReadClientCharacteristicConfigurationDescriptorResult ClientCharConfigDescriptorResult;

using GlobalCondition = std::function<bool()>;
static constexpr bool never() { return false; }

template <typename T>
static T await(IAsyncOperation<T> asyncInfo, GlobalCondition canceled = never, int timeout = 5000)
{
    QDeadlineTimer awaitTime(timeout);
    auto *dispatch = QAbstractEventDispatcher::instance();
    if (!dispatch)
        dispatch = QCoreApplication::eventDispatcher();
    do {
        QThread::yieldCurrentThread();
        if (asyncInfo.Status() != winrt::AsyncStatus::Started) {
            check_hresult(asyncInfo.ErrorCode());
            return asyncInfo.GetResults();
        }
        if (dispatch)
            dispatch->processEvents(QEventLoop::AllEvents);
    } while (!canceled() && !awaitTime.hasExpired());
    asyncInfo.Cancel();
    throw hresult_error(E_ABORT);
}

constexpr int timeout_infinity = -1;

template <typename T>
static inline T await_forever(IAsyncOperation<T> asyncInfo, GlobalCondition canceled = never)
{
    return await(asyncInfo, canceled, timeout_infinity);
}

#define LOG_HRESULT(hr) qCWarning(QT_BT_WINDOWS) << "HRESULT:" << quint32(hr)

#define HR(x) \
    std::invoke([&]() { \
        try { \
            x; \
        } catch (hresult_error const &e) { \
            LOG_HRESULT(e.code()) << "/*"  << #x << "*/"; \
            return e.code(); \
        } \
        return hresult{ S_OK } ; \
    })

#define TRY(x) \
    std::invoke([&]() { \
        try { \
            x; \
        } catch (hresult_error const &e) { \
            LOG_HRESULT(e.code()) << "/*"  << #x << "*/"; \
            return false; \
        } \
        return true; \
    })

#define SAFE(x) \
    std::invoke([&]() { \
        try { \
            return(x); \
        } catch (hresult_error const &e) { \
            LOG_HRESULT(e.code()) << "/*"  << #x << "*/"; \
            return decltype(x)(0); \
        } \
    })

#define WARN_AND_CONTINUE(msg) \
    { \
        qCWarning(QT_BT_WINDOWS) << msg; \
        continue; \
    }

#define DEC_CHAR_COUNT_AND_CONTINUE(msg) \
    { \
        qCWarning(QT_BT_WINDOWS) << msg; \
        --mCharacteristicsCountToBeDiscovered; \
        continue; \
    }

#define RETURN_SERVICE_ERROR(msg, service, error) \
    { \
        qCDebug(QT_BT_WINDOWS) << msg; \
        service->setError(error); \
        return; \
    }

#define RETURN_FALSE(msg) \
    { \
        qErrnoWarning(msg); \
        return false; \
    }

#define RETURN_MSG(msg) \
    { \
        qErrnoWarning(msg); \
        return; \
    }

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS)
Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINDOWS_SERVICE_THREAD)

static constexpr qint64 kMaxConnectTimeout = 20000; // 20 sec

static QByteArray byteArrayFromBuffer(IBuffer buffer, bool isWCharString = false)
{
    if (!buffer) {
        qErrnoWarning("nullptr passed to byteArrayFromBuffer");
        return QByteArray();
    }
    qsizetype size = SAFE(buffer.Length());
    if (!size)
        return QByteArray();
    if (isWCharString) {
        QString valueString = QString::fromUtf16(
            reinterpret_cast<char16_t *>(buffer.data())).left(size / 2);
        return valueString.toUtf8();
    }
    return QByteArray(reinterpret_cast<char *>(buffer.data()), size);
}

static QByteArray byteArrayFromGattResult(GattReadResult gattResult,
                                          bool isWCharString = false)
{
    auto buffer = SAFE(gattResult.Value());
    if (!buffer) {
        qCWarning(QT_BT_WINDOWS) << "Could not obtain buffer from GattReadResult";
        return QByteArray();
    }
    return byteArrayFromBuffer(buffer, isWCharString);
}

class QWinRTLowEnergyServiceHandler : public QObject
{
    Q_OBJECT
public:
    QWinRTLowEnergyServiceHandler(const QBluetoothUuid &service,
                                     GattDeviceService deviceService,
                                     QLowEnergyService::DiscoveryMode mode)
        : mService(service), mMode(mode), mDeviceService(deviceService)
    {
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    }

    ~QWinRTLowEnergyServiceHandler()
    {
        if (mDeviceService && mAbortRequested)
            TRY(mDeviceService.Close());
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    }

public slots:
    void obtainCharList()
    {
        auto exitCondition = [this]() { return mAbortRequested; };

        mIndicateChars.clear();
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;

        auto characteristicsResult
            = SAFE(await(mDeviceService.GetCharacteristicsAsync(), exitCondition));
        if (!characteristicsResult)
            return emitErrorAndQuitThread(QLatin1String("Could not obtain char list"));

        if (!SAFE(characteristicsResult.Status() == GattCommunicationStatus::Success))
            return emitErrorAndQuitThread(QLatin1String("Could not obtain char list"));

        auto characteristics = SAFE(characteristicsResult.Characteristics());
        if (!characteristics)
            return emitErrorAndQuitThread(QLatin1String("Could not obtain char list"));

        uint characteristicsCount = 0;
        if (!TRY(characteristicsCount = characteristics.Size()))
            return emitErrorAndQuitThread(QLatin1String("Could not obtain char list"));

        mCharacteristicsCountToBeDiscovered = characteristicsCount;
        for (uint i = 0; !mAbortRequested && (i < characteristicsCount); ++i) {
            auto characteristic = SAFE(characteristics.GetAt(i));
            if (!characteristic) {
                qCWarning(QT_BT_WINDOWS) << "Could not obtain characteristic at" << i;
                --mCharacteristicsCountToBeDiscovered;
                continue;
            }

            // For some strange reason, Windows doesn't discover descriptors of characteristics (if not paired).
            // Qt API assumes that all characteristics and their descriptors are discovered in one go.
            // So we start 'GetDescriptorsAsync' for each discovered characteristic and finish only
            // when GetDescriptorsAsync for all characteristics return.
            auto descResult = SAFE(await(characteristic.GetDescriptorsAsync(), exitCondition));
            if (!descResult)
                DEC_CHAR_COUNT_AND_CONTINUE("Could not obtain descriptor read result");

            uint16_t handle = 0;
            if (!TRY(handle = characteristic.AttributeHandle()))
                DEC_CHAR_COUNT_AND_CONTINUE("Could not obtain characteristic's attribute handle");

            QLowEnergyServicePrivate::CharData charData;
            charData.valueHandle = handle + 1;
            if (mStartHandle == 0 || mStartHandle > handle)
                mStartHandle = handle;
            if (mEndHandle == 0 || mEndHandle < handle)
                mEndHandle = handle;

            if (!TRY(charData.uuid = QBluetoothUuid(characteristic.Uuid())))
                DEC_CHAR_COUNT_AND_CONTINUE("Could not read characteristic UUID");

            GattCharacteristicProperties properties;
            if (!TRY(properties = characteristic.CharacteristicProperties()))
                DEC_CHAR_COUNT_AND_CONTINUE("Could not read characteristic properties");

            charData.properties = QLowEnergyCharacteristic::PropertyTypes(static_cast<uint32_t>(properties) & 0xff);
            if (charData.properties & QLowEnergyCharacteristic::Read
                && mMode == QLowEnergyService::FullDiscovery) {

                GattReadResult readResult = nullptr;
                if (!TRY(readResult = await(characteristic.ReadValueAsync(BluetoothCacheMode::Uncached), exitCondition)))
                    DEC_CHAR_COUNT_AND_CONTINUE("Could not read characteristic");

                if (!readResult)
                    qCWarning(QT_BT_WINDOWS) << "Characteristic read result is null";
                else
                    charData.value = byteArrayFromGattResult(readResult);
            }
            mCharacteristicList.insert(handle, charData);

            if (!SAFE(descResult.Status() == GattCommunicationStatus::Success))
                DEC_CHAR_COUNT_AND_CONTINUE("Descriptor operation failed");

            auto descriptors = SAFE(descResult.Descriptors());
            if (!descriptors)
                DEC_CHAR_COUNT_AND_CONTINUE("Could not obtain list of descriptors");
            uint descriptorCount;
            if (!TRY(descriptorCount = descriptors.Size()))
                DEC_CHAR_COUNT_AND_CONTINUE("Could not obtain list of descriptors' size");
            for (uint j = 0; !mAbortRequested && (j < descriptorCount); ++j) {
                QLowEnergyServicePrivate::DescData descData;
                GattDescriptor descriptor = SAFE(descriptors.GetAt(j));
                if (!descriptor)
                    WARN_AND_CONTINUE("Could not access descriptor");
                quint16 descHandle;
                if (!TRY(descHandle = descriptor.AttributeHandle()))
                    WARN_AND_CONTINUE("Could not get descriptor handle");
                if (!TRY(descData.uuid = QBluetoothUuid(descriptor.Uuid())))
                    WARN_AND_CONTINUE("Could not get descriptor UUID");
                charData.descriptorList.insert(descHandle, descData);
                if (descData.uuid == QBluetoothUuid(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration)) {
                    if (mMode == QLowEnergyService::FullDiscovery) {
                        auto readResult = SAFE(await(characteristic.ReadClientCharacteristicConfigurationDescriptorAsync(), exitCondition));
                        if (!readResult)
                            WARN_AND_CONTINUE("Could not read descriptor value");
                        GattClientCharacteristicConfigurationDescriptorValue value;
                        if (!TRY(value = readResult.ClientCharacteristicConfigurationDescriptor()))
                            WARN_AND_CONTINUE("Could not get descriptor value from result");
                        quint16 result = 0;
                        bool correct = false;
                        if (value & GattClientCharacteristicConfigurationDescriptorValue::Indicate) {
                            result |= GattClientCharacteristicConfigurationDescriptorValue::Indicate;
                            correct = true;
                        }
                        if (value & GattClientCharacteristicConfigurationDescriptorValue::Notify) {
                            result |= GattClientCharacteristicConfigurationDescriptorValue::Notify;
                            correct = true;
                        }
                        if (value == GattClientCharacteristicConfigurationDescriptorValue::None)
                            correct = true;
                        if (!correct)
                            continue;

                        descData.value = QByteArray(2, Qt::Uninitialized);
                        qToLittleEndian(result, descData.value.data());
                    }
                    mIndicateChars << charData.uuid;
                } else {
                    if (mMode == QLowEnergyService::FullDiscovery) {

                        auto readResult = SAFE(await(descriptor.ReadValueAsync(BluetoothCacheMode::Uncached), exitCondition));
                        if (!readResult)
                            WARN_AND_CONTINUE("Could not read descriptor value");
                        descData.value = byteArrayFromGattResult(readResult);
                    }
                }
                charData.descriptorList.insert(descHandle, descData);
            }

            mCharacteristicList.insert(handle, charData);
            --mCharacteristicsCountToBeDiscovered;
        }
        checkAllCharacteristicsDiscovered();
    }

    void setAbortRequested()
    {
        mAbortRequested = true;
    }

private:
    void checkAllCharacteristicsDiscovered();
    void emitErrorAndQuitThread(const QString &error);

public:
    QBluetoothUuid mService;
    QLowEnergyService::DiscoveryMode mMode;
    GattDeviceService mDeviceService;
    QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> mCharacteristicList;
    uint mCharacteristicsCountToBeDiscovered = 0;
    quint16 mStartHandle = 0;
    quint16 mEndHandle = 0;
    QList<QBluetoothUuid> mIndicateChars;
    bool mAbortRequested = false;

signals:
    void charListObtained(const QBluetoothUuid &service,
                          QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> charList,
                          QList<QBluetoothUuid> indicateChars, QLowEnergyHandle startHandle,
                          QLowEnergyHandle endHandle);
    void errorOccured(const QString &error);
};

void QWinRTLowEnergyServiceHandler::checkAllCharacteristicsDiscovered()
{
    if (!mAbortRequested && (mCharacteristicsCountToBeDiscovered == 0)) {
        emit charListObtained(mService, mCharacteristicList, mIndicateChars,
                              mStartHandle, mEndHandle);
    }
    QThread::currentThread()->quit();
}

void QWinRTLowEnergyServiceHandler::emitErrorAndQuitThread(const QString &error)
{
    mAbortRequested = true; // so that the service is closed during cleanup
    emit errorOccured(error);
    QThread::currentThread()->quit();
}

class QWinRTLowEnergyConnectionHandler : public QObject
{
    Q_OBJECT
public:
    explicit QWinRTLowEnergyConnectionHandler(const QBluetoothAddress &address) : mAddress(address)
    {
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
        // This should be checked before the handler is created
        Q_ASSERT(!mAddress.isNull());
    }
    ~QWinRTLowEnergyConnectionHandler()
    {
        qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
        mDevice = nullptr;
        mGattSession = nullptr;
        // To close the COM library gracefully, each successful call to
        // CoInitialize, including those that return S_FALSE, must be balanced
        // by a corresponding call to CoUninitialize.
        if (mComInitialized)
            winrt::uninit_apartment();
    }

public slots:
    void connectToDevice();
    void handleDeviceDisconnectRequest();

signals:
    void deviceConnected(com_ptr<abi::BluetoothLEDevice> device, com_ptr<abi::GattSession> session);
    void errorOccurred(const QString &error);

private:
    void connectToPairedDevice();
    void connectToUnpairedDevice();
    void emitErrorAndQuitThread(const QString &error);
    void emitErrorAndQuitThread(const char *error);
    void emitConnectedAndQuitThread();

    BluetoothLEDevice mDevice = nullptr;
    GattSession mGattSession = nullptr;
    const QBluetoothAddress mAddress;
    bool mAbortConnection = false;
    bool mComInitialized = false;
};

void QWinRTLowEnergyConnectionHandler::connectToDevice()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    mComInitialized = TRY(winrt::init_apartment());

    auto earlyExit = [this]() { return mAbortConnection; };
    if (!(mDevice = SAFE(await(BluetoothLEDevice::FromBluetoothAddressAsync(mAddress.toUInt64()), earlyExit))))
        return emitErrorAndQuitThread("Could not find LE device from address");

    // get GattSession: 1. get device id
    BluetoothDeviceId deviceId = SAFE(mDevice.BluetoothDeviceId());
    if (!deviceId)
        return emitErrorAndQuitThread("Could not get device id");

    // get GattSession: 3. get session
    if (!(mGattSession = SAFE(await(GattSession::FromDeviceIdAsync(deviceId), earlyExit))))
        return emitErrorAndQuitThread("Could not get GattSession from id");

    BluetoothConnectionStatus status;
    if (!TRY(status = mDevice.ConnectionStatus()))
        return emitErrorAndQuitThread("Could not get connection status");
    if (status == BluetoothConnectionStatus::Connected)
        return emitConnectedAndQuitThread();

    QBluetoothLocalDevice localDevice;
    QBluetoothLocalDevice::Pairing pairing = localDevice.pairingStatus(mAddress);
    if (pairing == QBluetoothLocalDevice::Unpaired)
        connectToUnpairedDevice();
    else
        connectToPairedDevice();
}

void QWinRTLowEnergyConnectionHandler::handleDeviceDisconnectRequest()
{
    mAbortConnection = true;
    // Disconnect from the QLowEnergyControllerPrivateWinRT, so that it does
    // not get notifications. It's absolutely fine to keep doing smth in
    // background, as multiple connections to the same device should be handled
    // correctly by OS.
    disconnect(this, &QWinRTLowEnergyConnectionHandler::deviceConnected, nullptr, nullptr);
    disconnect(this, &QWinRTLowEnergyConnectionHandler::errorOccurred, nullptr, nullptr);
}

void QWinRTLowEnergyConnectionHandler::connectToPairedDevice()
{
    auto earlyExit = [this]() { return mAbortConnection; };
    QDeadlineTimer deadline(kMaxConnectTimeout);
    while (!mAbortConnection && !deadline.hasExpired()) {

        auto deviceServicesResult = SAFE(await(mDevice.GetGattServicesAsync(), earlyExit));
        if (!deviceServicesResult)
            return emitErrorAndQuitThread("Could not obtain services");

        if (!SAFE(deviceServicesResult.Status() == GattCommunicationStatus::Success))
            return emitErrorAndQuitThread("Service operation failed");

        auto deviceServices = SAFE(deviceServicesResult.Services());
        if (!deviceServices)
            return emitErrorAndQuitThread("Could not obtain list of services");

        uint serviceCount;
        if (!TRY(serviceCount = deviceServices.Size()))
            return emitErrorAndQuitThread("Could not obtain size of list of services");
        if (serviceCount == 0)
            return emitErrorAndQuitThread("Found devices without services");

        // Windows automatically connects to the device as soon as a service value is read/written.
        // Thus we read one value in order to establish the connection.
        for (uint i = 0; i < serviceCount; ++i) {

            auto service = SAFE(deviceServices.GetAt(i));
            if (!service)
                return emitErrorAndQuitThread("Could not obtain service");

            auto characteristicsResult = SAFE(await(service.GetCharacteristicsAsync(), earlyExit));
            if (!characteristicsResult)
                return emitErrorAndQuitThread("Could not obtain characteristic");

            if (!SAFE(characteristicsResult.Status() == GattCommunicationStatus::Success)) {
                qCWarning(QT_BT_WINDOWS) << "Characteristic operation failed";
                break;
            }

            IVectorView<GattCharacteristic> characteristics = nullptr;
            auto hr = HR(characteristics = characteristicsResult.Characteristics());
            if (hr == E_ACCESSDENIED) {
                // Everything will work as expected up until this point if the
                // manifest capabilties for bluetooth LE are not set.
                return emitErrorAndQuitThread("Could not obtain characteristic list. "
                    "Please check your manifest capabilities");
            } else if (FAILED(hr) || !characteristics) {
                return emitErrorAndQuitThread("Could not obtain characteristic list.");
            }

            uint characteristicsCount;
            if (!TRY(characteristicsCount = characteristics.Size()))
                return emitErrorAndQuitThread("Could not obtain size of characteristic list.");
            for (uint j = 0; j < characteristicsCount; ++j) {

                auto characteristic = SAFE(characteristics.GetAt(j));
                if (!characteristic)
                    return emitErrorAndQuitThread("Could not get characteristic");
                GattReadResult result = nullptr;
                hr = HR(result = await(
                    characteristic.ReadValueAsync(BluetoothCacheMode::Uncached), earlyExit));
                if (hr == E_ILLEGAL_METHOD_CALL) {
                    // E_ILLEGAL_METHOD_CALL will be the result for a device, that is not reachable at
                    // the moment. In this case we should jump back into the outer loop and keep trying.
                    break;
                } else if (FAILED(hr) || !result) {
                    return emitErrorAndQuitThread("Could not read characteristic value");
                }

                auto buffer = SAFE(result.Value());
                if (!buffer) {
                    qCDebug(QT_BT_WINDOWS) << "Problem reading value";
                    break;
                }

                emitConnectedAndQuitThread();
                return;
            }
        }
    }
    // If we got here because of mAbortConnection == true, the error message
    // will not be delivered, so it does not matter. But we need to terminate
    // the thread anyway!
    emitErrorAndQuitThread("Connect to device failed due to timeout!");
}

void QWinRTLowEnergyConnectionHandler::connectToUnpairedDevice()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;

    auto earlyExit = [this]() { return mAbortConnection; };
    QDeadlineTimer deadline(kMaxConnectTimeout);
    while (!mAbortConnection && !deadline.hasExpired()) {

        auto deviceServicesResult = SAFE(await_forever(mDevice.GetGattServicesAsync(), earlyExit));
        if (!deviceServicesResult)
            return emitErrorAndQuitThread("Could not obtain services");

        GattCommunicationStatus commStatus;
        if (!TRY(commStatus = deviceServicesResult.Status()))
            return emitErrorAndQuitThread("Could not obtain comm status");
        if (commStatus == GattCommunicationStatus::Unreachable)
            continue;
        if (commStatus != GattCommunicationStatus::Success)
            return emitErrorAndQuitThread("Service operation failed");

        emitConnectedAndQuitThread();
        return;
    }
    // If we got here because of mAbortConnection == true, the error message
    // will not be delivered, so it does not matter. But we need to terminate
    // the thread anyway!
    emitErrorAndQuitThread("Connect to device failed due to timeout!");
}

void QWinRTLowEnergyConnectionHandler::emitErrorAndQuitThread(const QString &error)
{
    emit errorOccurred(error);
    QThread::currentThread()->quit();
}

void QWinRTLowEnergyConnectionHandler::emitErrorAndQuitThread(const char *error)
{
    emitErrorAndQuitThread(QString::fromUtf8(error));
}

void QWinRTLowEnergyConnectionHandler::emitConnectedAndQuitThread()
{
    emit deviceConnected(mDevice.as<abi::BluetoothLEDevice>(), mGattSession.as<abi::GattSession>());
    QThread::currentThread()->quit();
}

QLowEnergyControllerPrivateWinRT::QLowEnergyControllerPrivateWinRT()
    : QLowEnergyControllerPrivate()
{
    registerQLowEnergyControllerMetaType();
    connect(this, &QLowEnergyControllerPrivateWinRT::characteristicChanged,
            this, &QLowEnergyControllerPrivateWinRT::handleCharacteristicChanged,
            Qt::QueuedConnection);
}

QLowEnergyControllerPrivateWinRT::~QLowEnergyControllerPrivateWinRT()
{
    unregisterFromStatusChanges();
    unregisterFromValueChanges();
}

void QLowEnergyControllerPrivateWinRT::init()
{
}

void QLowEnergyControllerPrivateWinRT::connectToDevice()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    if (remoteDevice.isNull()) {
        qCWarning(QT_BT_WINDOWS) << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }
    setState(QLowEnergyController::ConnectingState);

    QWinRTLowEnergyConnectionHandler *worker = new QWinRTLowEnergyConnectionHandler(remoteDevice);
    QThread *thread = new QThread;
    worker->moveToThread(thread);
    connect(this, &QLowEnergyControllerPrivateWinRT::abortConnection, worker,
            &QWinRTLowEnergyConnectionHandler::handleDeviceDisconnectRequest);
    connect(thread, &QThread::started, worker, &QWinRTLowEnergyConnectionHandler::connectToDevice);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QObject::destroyed, thread, &QObject::deleteLater);
    connect(worker, &QWinRTLowEnergyConnectionHandler::errorOccurred, this,
            [this](const QString &msg) { handleConnectionError(msg.toUtf8().constData()); });
    connect(worker, &QWinRTLowEnergyConnectionHandler::deviceConnected, this,
            [this](com_ptr<abi::BluetoothLEDevice> device, com_ptr<abi::GattSession> session) {
                if (!device || !session) {
                    handleConnectionError("Failed to get device or gatt service");
                    return;
                }
                mDevice = device.as<BluetoothLEDevice>();
                mGattSession = session.as<GattSession>();

                if (!registerForStatusChanges() || !registerForMtuChanges()) {
                    handleConnectionError("Failed to register for changes");
                    return;
                }

                Q_Q(QLowEnergyController);
                setState(QLowEnergyController::ConnectedState);
                emit q->connected();
            });
    thread->start();
}

void QLowEnergyControllerPrivateWinRT::disconnectFromDevice()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    Q_Q(QLowEnergyController);
    setState(QLowEnergyController::ClosingState);
    emit abortConnection();
    unregisterFromValueChanges();
    unregisterFromStatusChanges();
    unregisterFromMtuChanges();
    clearAllServices();
    mGattSession = nullptr;
    mDevice = nullptr;
    setState(QLowEnergyController::UnconnectedState);
    emit q->disconnected();
}

bool QLowEnergyControllerPrivateWinRT::getNativeService(const QBluetoothUuid &serviceUuid,
                                                           NativeServiceCallback callback)
{
    if (m_openedServices.contains(serviceUuid)) {
        callback(m_openedServices.value(serviceUuid, nullptr));
        return true;
    }

    auto servicesResultOperation = SAFE(mDevice.GetGattServicesForUuidAsync(GUID(serviceUuid)));
    if (!servicesResultOperation)
        RETURN_FALSE("Could not start async services request");

    QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
    bool ok = TRY(servicesResultOperation.Completed(
        [thisPtr, callback, &serviceUuid](
            IAsyncOperation<GattDeviceServicesResult> const &op, winrt::AsyncStatus const status)
        {
            if (!thisPtr) {
                qCWarning(QT_BT_WINDOWS) << "LE controller was removed while getting native service";
                return;
            }

            if (status != winrt::AsyncStatus::Completed) {
                qCDebug(QT_BT_WINDOWS) << "Failed to get result of async service request";
                return;
            }

            auto result = SAFE(op.GetResults());
            if (!result)
                RETURN_MSG("Failed to get result of async service request");

            auto services = SAFE(result.Services());
            if (!services)
                RETURN_MSG("Failed to extract services from the result");

            uint servicesCount = 0;
            if (!TRY(servicesCount = services.Size()))
                RETURN_MSG("Failed to extract services count");

            if (servicesCount > 0) {
                if (servicesCount > 1) {
                    qWarning() << "getNativeService: more than one service detected for UUID"
                        << serviceUuid << "The first service will be used.";
                }

                auto service = SAFE(services.GetAt(0));
                if (!service) {
                    qCDebug(QT_BT_WINDOWS) << "Could not obtain native service for Uuid"
                        << serviceUuid;
                } else {
                    thisPtr->m_openedServices.insert(serviceUuid, service);
                    callback(service); // Use the service in a custom callback
                }
            } else {
                qCWarning(QT_BT_WINDOWS) << "No services found for Uuid" << serviceUuid;
            }
        }));

    return ok;
}

bool QLowEnergyControllerPrivateWinRT::getNativeCharacteristic(
        const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid,
        NativeCharacteristicCallback callback)
{
    QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
    auto serviceCallback = [thisPtr, callback, charUuid](GattDeviceService service) {

        auto characteristicRequestOp = SAFE(service.GetCharacteristicsForUuidAsync(GUID(charUuid)));
        if (!characteristicRequestOp)
            RETURN_MSG("Could not start async characteristics request");

        TRY(characteristicRequestOp.Completed(
            [thisPtr, callback](
                IAsyncOperation<GattCharacteristicsResult> const &op, winrt::AsyncStatus const status)
            {
                if (!thisPtr)
                    return;

                if (status != winrt::AsyncStatus::Completed) {
                    qCDebug(QT_BT_WINDOWS) << "Failed to get result of async characteristic "
                                              "operation";
                    return;
                }

                auto result = SAFE(op.GetResults());
                if (!result)
                    RETURN_MSG("Failed to get result of async characteristic operation");

                GattCommunicationStatus commStatus;
                if (!TRY(commStatus = result.Status()) || commStatus != GattCommunicationStatus::Success) {
                    qErrnoWarning("Native characteristic operation failed.");
                    return;
                }

                auto characteristics = SAFE(result.Characteristics());
                if (!characteristics)
                    RETURN_MSG("Could not obtain characteristic list.");

                uint size;
                if (!TRY(size = characteristics.Size()))
                    RETURN_MSG("Could not obtain characteristic list's size.");

                if (size != 1)
                    qErrnoWarning("More than 1 characteristic found.");

                auto characteristic = SAFE(characteristics.GetAt(0));
                if (!characteristic)
                    RETURN_MSG("Could not obtain first characteristic for service");

                callback(characteristic); // use the characteristic in a custom callback
            }));
    };

    if (!getNativeService(serviceUuid, serviceCallback)) {
        qCDebug(QT_BT_WINDOWS) << "Failed to get native service for" << serviceUuid;
        return false;
    }

    return true;
}

void QLowEnergyControllerPrivateWinRT::registerForValueChanges(const QBluetoothUuid &serviceUuid,
                                                                  const QBluetoothUuid &charUuid)
{
    qCDebug(QT_BT_WINDOWS) << "Registering characteristic" << charUuid << "in service"
                         << serviceUuid << "for value changes";
    for (const ValueChangedEntry &entry : std::as_const(mValueChangedTokens)) {
        GUID guuid{ 0 };
        if (!TRY(guuid = entry.characteristic.Uuid()))
            WARN_AND_CONTINUE("Could not obtain characteristic's Uuid");
        if (QBluetoothUuid(guuid) == charUuid)
            return;
    }

    auto callback = [this, charUuid, serviceUuid](GattCharacteristic characteristic) {
        winrt::event_token token;
        if (!TRY(token = characteristic.ValueChanged({ this, &QLowEnergyControllerPrivateWinRT::onValueChange })))
            RETURN_MSG("Could not register characteristic for value changes");

        mValueChangedTokens.append(ValueChangedEntry(characteristic, token));
        qCDebug(QT_BT_WINDOWS) << "Characteristic" << charUuid << "in service"
                               << serviceUuid << "registered for value changes";
    };

    if (!getNativeCharacteristic(serviceUuid, charUuid, callback)) {
        qCDebug(QT_BT_WINDOWS).nospace() << "Could not obtain native characteristic "
                                         << charUuid << " from service " << serviceUuid
                                         << ". Qt will not be able to signal"
                                         << " changes for this characteristic.";
    }
}

void QLowEnergyControllerPrivateWinRT::unregisterFromValueChanges()
{
    qCDebug(QT_BT_WINDOWS) << "Unregistering " << mValueChangedTokens.size() << " value change tokens";
    for (const ValueChangedEntry &entry : std::as_const(mValueChangedTokens)) {
        if (!entry.characteristic) {
            qCWarning(QT_BT_WINDOWS) << "Unregistering from value changes for characteristic failed."
                                   << "Characteristic has been deleted";
            continue;
        }
        if (!TRY(entry.characteristic.ValueChanged(entry.token)))
            qCWarning(QT_BT_WINDOWS) << "Unregistering from value changes for characteristic failed.";
    }
    mValueChangedTokens.clear();
}

void QLowEnergyControllerPrivateWinRT::onValueChange(GattCharacteristic characteristic, GattValueChangedEventArgs const &args)
{
    quint16 handle = 0;
    if (!TRY(handle = characteristic.AttributeHandle()))
        RETURN_MSG("Could not obtain characteristic's handle");

    auto buffer = SAFE(args.CharacteristicValue());
    if (!buffer)
        RETURN_MSG("Could not obtain characteristic's value");

    emit characteristicChanged(handle, byteArrayFromBuffer(buffer));
}

bool QLowEnergyControllerPrivateWinRT::registerForMtuChanges()
{
    if (!mDevice || !mGattSession)
        return false;
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    if (!TRY(mMtuChangedToken = mGattSession.MaxPduSizeChanged({ this, &QLowEnergyControllerPrivateWinRT::onMtuChange })))
        RETURN_FALSE("Could not add MTU callback");
    return true;
}

void QLowEnergyControllerPrivateWinRT::unregisterFromMtuChanges()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    if (mDevice && mGattSession && mMtuChangedToken) {
        TRY(mGattSession.MaxPduSizeChanged(mMtuChangedToken));
        mMtuChangedToken = { 0 };
    }
}

void QLowEnergyControllerPrivateWinRT::onMtuChange(GattSession session, winrt::IInspectable args)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;

    if (session != mGattSession) {
        qCWarning(QT_BT_WINDOWS) << "Got MTU changed event for wrong or outdated GattSession.";
    } else {
        Q_Q(QLowEnergyController);
        emit q->mtuChanged(mtu());
    }
}

bool QLowEnergyControllerPrivateWinRT::registerForStatusChanges()
{
    if (!mDevice)
        return false;

    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;

    if (!TRY(mStatusChangedToken = mDevice.ConnectionStatusChanged({ this, &QLowEnergyControllerPrivateWinRT::onStatusChange })))
        RETURN_FALSE("Could not add status callback");
    return true;
}

void QLowEnergyControllerPrivateWinRT::unregisterFromStatusChanges()
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__;
    if (mDevice && mStatusChangedToken) {
        TRY(mDevice.ConnectionStatusChanged(mStatusChangedToken));
        mStatusChangedToken = { 0 };
    }
}

void QLowEnergyControllerPrivateWinRT::onStatusChange(BluetoothLEDevice dev, winrt::IInspectable args)
{
    Q_Q(QLowEnergyController);

    BluetoothConnectionStatus status;
    if (!TRY(status = dev.ConnectionStatus()))
        RETURN_MSG("Could not obtain connection status");

    if (state == QLowEnergyController::ConnectingState
        && status == BluetoothConnectionStatus::Connected) {
        setState(QLowEnergyController::ConnectedState);
        emit q->connected();
    } else if (state != QLowEnergyController::UnconnectedState
        && status == BluetoothConnectionStatus::Disconnected) {
        invalidateServices();
        unregisterFromValueChanges();
        unregisterFromStatusChanges();
        unregisterFromMtuChanges();
        mGattSession = nullptr;
        mDevice = nullptr;
        setError(QLowEnergyController::RemoteHostClosedError);
        setState(QLowEnergyController::UnconnectedState);
        emit q->disconnected();
    }
}

void QLowEnergyControllerPrivateWinRT::obtainIncludedServices(QSharedPointer<QLowEnergyServicePrivate> servicePointer, GattDeviceService service)
{
    Q_Q(QLowEnergyController);

    auto result = SAFE(await(service.GetIncludedServicesAsync()));
    if (!result)
        RETURN_MSG("Could not obtain included services");

    // The device can be disconnected by the time we return from await()
    if (state != QLowEnergyController::DiscoveringState)
        return;

    GattCommunicationStatus status;
    if (!TRY(status = result.Status())) {
        qErrnoWarning("Could not obtain list of included services");
        return;
    }

    auto includedServices = SAFE(result.Services());
    if (!includedServices)
        RETURN_MSG("Could not obtain service list");

    uint count;
    if (!TRY(count = includedServices.Size()))
        RETURN_MSG("Could not obtain service list's size");

    for (uint i = 0; i < count; ++i) {

        auto includedService = SAFE(includedServices.GetAt(i));
        if (!includedService)
            WARN_AND_CONTINUE("Could not obtain service from list");

        GUID guuid;
        if (!TRY(guuid = includedService.Uuid()))
            WARN_AND_CONTINUE("Could not obtain included service's Uuid");

        const QBluetoothUuid includedUuid(guuid);
        QSharedPointer<QLowEnergyServicePrivate> includedPointer;
        qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__
                                            << "Changing service pointer from thread"
                                            << QThread::currentThread();
        if (serviceList.contains(includedUuid)) {
            includedPointer = serviceList.value(includedUuid);
        } else {
            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = includedUuid;
            priv->setController(this);

            includedPointer = QSharedPointer<QLowEnergyServicePrivate>(priv);
            serviceList.insert(includedUuid, includedPointer);
        }
        includedPointer->type |= QLowEnergyService::IncludedService;
        servicePointer->includedServices.append(includedUuid);

        obtainIncludedServices(includedPointer, includedService);

        emit q->serviceDiscovered(includedUuid);
    }
}

void QLowEnergyControllerPrivateWinRT::onServiceDiscoveryFinished(IAsyncOperation<GattDeviceServicesResult> const &op, winrt::AsyncStatus status)
{
    // Check if the device is in the proper state, because it can already be
    // disconnected when the callback arrives.
    // Also the callback can theoretically come when the connection is
    // reestablisheed again (for example, if the user quickly clicks
    // "Disconnect" and then "Connect" again in some UI). But we can probably
    // omit such details, as we are connecting to the same device anyway.
    if (state != QLowEnergyController::DiscoveringState)
        return;

    Q_Q(QLowEnergyController);
    if (status != winrt::AsyncStatus::Completed) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain services";
        return;
    }

    auto result = SAFE(op.GetResults());
    if (!result)
        return handleConnectionError("Could not obtain service discovery result");

    GattCommunicationStatus commStatus;
    if (!TRY(commStatus = result.Status()))
        return handleConnectionError("Could not obtain service discovery status");

    if (commStatus != GattCommunicationStatus::Success)
        return;

    auto deviceServices = SAFE(result.Services());
    if (!deviceServices)
        return handleConnectionError("Could not obtain service list");

    uint serviceCount;
    if (!TRY(serviceCount = deviceServices.Size()))
        return handleConnectionError("Could not obtain service list size");

    for (uint i = 0; i < serviceCount; ++i) {

        auto deviceService = SAFE(deviceServices.GetAt(i));
        if (!deviceService)
            WARN_AND_CONTINUE("Could not obtain service");

        GUID guuid;
        if (!TRY(guuid = deviceService.Uuid()))
            WARN_AND_CONTINUE("Could not obtain service's Uuid");

        const QBluetoothUuid service(guuid);
        m_openedServices.insert(service, deviceService);

        qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__
                                            << "Changing service pointer from thread"
                                            << QThread::currentThread();
        QSharedPointer<QLowEnergyServicePrivate> pointer;
        if (serviceList.contains(service)) {
            pointer = serviceList.value(service);
        } else {
            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = service;
            priv->setController(this);

            pointer = QSharedPointer<QLowEnergyServicePrivate>(priv);
            serviceList.insert(service, pointer);
        }
        pointer->type |= QLowEnergyService::PrimaryService;

        obtainIncludedServices(pointer, deviceService);
        // The obtainIncludedServices method calls await(), so the device can be
        // disconnected by the time we return from it. TODO - rewrite in an
        // async way!
        if (state != QLowEnergyController::DiscoveringState) {
            emit q->discoveryFinished(); // Probably not needed when the device
                                         // is already disconnected?
            return;
        }

        emit q->serviceDiscovered(service);
    }

    setState(QLowEnergyController::DiscoveredState);
    emit q->discoveryFinished();
}

void QLowEnergyControllerPrivateWinRT::clearAllServices()
{
    // These services will be closed in the respective
    // QWinRTLowEnergyServiceHandler workers (in background threads).
    for (auto &uuid : m_requestDetailsServiceUuids)
        m_openedServices.remove(uuid);
    m_requestDetailsServiceUuids.clear();

    for (auto service : m_openedServices)
        TRY(service.Close());
    m_openedServices.clear();
}

void QLowEnergyControllerPrivateWinRT::closeAndRemoveService(const QBluetoothUuid &uuid)
{
    auto record = m_openedServices.find(uuid);
    if (record != m_openedServices.end()) {
        auto service = record.value();
        m_openedServices.erase(record);
        if (service)
            TRY(service.Close());
    }
}

void QLowEnergyControllerPrivateWinRT::discoverServices()
{
    qCDebug(QT_BT_WINDOWS) << "Service discovery initiated";
    // clear the previous services cache, as we request the services again
    clearAllServices();

    auto asyncResult = SAFE(mDevice.GetGattServicesAsync());
    if (!asyncResult)
        return handleConnectionError("Could not obtain services");

    if (!TRY(asyncResult.Completed({ this, &QLowEnergyControllerPrivateWinRT::onServiceDiscoveryFinished })))
        return handleConnectionError("Could not register services discovery callback");
}

void QLowEnergyControllerPrivateWinRT::discoverServiceDetails(
        const QBluetoothUuid &service, QLowEnergyService::DiscoveryMode mode)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service;
    if (!serviceList.contains(service)) {
        qCWarning(QT_BT_WINDOWS) << "Discovery done of unknown service:"
            << service.toString();
        return;
    }

    // clear the cache to rediscover service details
    closeAndRemoveService(service);

    auto serviceCallback = [service, mode, this](GattDeviceService deviceService) {
        discoverServiceDetailsHelper(service, mode, deviceService);
    };

    if (!getNativeService(service, serviceCallback))
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native service for uuid " << service;
}

void QLowEnergyControllerPrivateWinRT::discoverServiceDetailsHelper(
        const QBluetoothUuid &service, QLowEnergyService::DiscoveryMode mode,
        GattDeviceService deviceService)
{
    auto reactOnDiscoveryError = [](QSharedPointer<QLowEnergyServicePrivate> service,
                                    const auto &msg)
    {
        qCDebug(QT_BT_WINDOWS) << msg;
        service->setError(QLowEnergyService::UnknownError);
        service->setState(QLowEnergyService::RemoteService);
    };
    //update service data
    QSharedPointer<QLowEnergyServicePrivate> pointer = serviceList.value(service);
    if (!pointer) {
        qCDebug(QT_BT_WINDOWS) << "Device was disconnected while doing service discovery";
        return;
    }
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                          << QThread::currentThread();
    pointer->setState(QLowEnergyService::RemoteServiceDiscovering);

    auto result = SAFE(await_forever(deviceService.GetIncludedServicesAsync()));
    if (!result)
        return reactOnDiscoveryError(pointer, "Could not obtain included service list");

    if (SAFE(result.Status() != GattCommunicationStatus::Success))
        return reactOnDiscoveryError(pointer, "Obtaining list of included services failed");

    auto deviceServices = SAFE(result.Services());
    if (!deviceServices)
        return reactOnDiscoveryError(pointer, "Could not obtain service list from result");

    uint serviceCount;
    if (!TRY(serviceCount = deviceServices.Size()))
        return reactOnDiscoveryError(pointer, "Could not obtain included service list's size");

    for (uint i = 0; i < serviceCount; ++i) {

        auto includedService = SAFE(deviceServices.GetAt(i));
        if (!includedService)
            WARN_AND_CONTINUE("Could not obtain service from list");

        GUID guuid;
        if (!TRY(guuid = includedService.Uuid()))
            WARN_AND_CONTINUE("Could not obtain service Uuid");

        const QBluetoothUuid service(guuid);
        if (service.isNull()) {
            qCDebug(QT_BT_WINDOWS) << "Could not find service";
            continue;
        }

        pointer->includedServices.append(service);

        // update the type of the included service
        QSharedPointer<QLowEnergyServicePrivate> otherService = serviceList.value(service);
        if (!otherService.isNull())
            otherService->type |= QLowEnergyService::IncludedService;
    }

    QWinRTLowEnergyServiceHandler *worker =
            new QWinRTLowEnergyServiceHandler(service, deviceService, mode);
    m_requestDetailsServiceUuids.insert(service);
    QThread *thread = new QThread;
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &QWinRTLowEnergyServiceHandler::obtainCharList);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QObject::destroyed, thread, &QObject::deleteLater);
    connect(this, &QLowEnergyControllerPrivateWinRT::abortConnection,
            worker, &QWinRTLowEnergyServiceHandler::setAbortRequested);
    connect(worker, &QWinRTLowEnergyServiceHandler::errorOccured,
            this, &QLowEnergyControllerPrivateWinRT::handleServiceHandlerError);
    connect(worker, &QWinRTLowEnergyServiceHandler::charListObtained, this,
            [this](const QBluetoothUuid &service, QHash<QLowEnergyHandle,
            QLowEnergyServicePrivate::CharData> charList, QList<QBluetoothUuid> indicateChars,
            QLowEnergyHandle startHandle, QLowEnergyHandle endHandle) {
        if (!serviceList.contains(service)) {
            qCWarning(QT_BT_WINDOWS)
                    << "Discovery complete for unknown service:" << service.toString();
            return;
        }
        m_requestDetailsServiceUuids.remove(service);

        QSharedPointer<QLowEnergyServicePrivate> pointer = serviceList.value(service);
        pointer->startHandle = startHandle;
        pointer->endHandle = endHandle;
        pointer->characteristicList = charList;

        for (const QBluetoothUuid &indicateChar : std::as_const(indicateChars))
            registerForValueChanges(service, indicateChar);

        pointer->setState(QLowEnergyService::RemoteServiceDiscovered);
    });
    thread->start();
}

void QLowEnergyControllerPrivateWinRT::startAdvertising(
        const QLowEnergyAdvertisingParameters &,
        const QLowEnergyAdvertisingData &,
        const QLowEnergyAdvertisingData &)
{
    setError(QLowEnergyController::AdvertisingError);
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::stopAdvertising()
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::requestConnectionUpdate(const QLowEnergyConnectionParameters &)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::readCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service << charHandle;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::CharacteristicReadError);
        Q_UNIMPLEMENTED();
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINDOWS) << charHandle << "could not be found in service" << service->uuid;
        service->setError(QLowEnergyService::CharacteristicReadError);
        return;
    }

    const auto charData = service->characteristicList.value(charHandle);
    if (!(charData.properties & QLowEnergyCharacteristic::Read))
        qCDebug(QT_BT_WINDOWS) << "Read flag is not set for characteristic" << charData.uuid;

    auto characteristicCallback = [charHandle, service, this](
            GattCharacteristic characteristic) {
        readCharacteristicHelper(service, charHandle, characteristic);
    };

    if (!getNativeCharacteristic(service->uuid, charData.uuid, characteristicCallback)) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native characteristic" << charData.uuid
                               << "from service" << service->uuid;
        service->setError(QLowEnergyService::CharacteristicReadError);
    }
}

void QLowEnergyControllerPrivateWinRT::readCharacteristicHelper(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        GattCharacteristic characteristic)
{
    auto readOp = SAFE(characteristic.ReadValueAsync(BluetoothCacheMode::Uncached));
    if (!readOp)
        RETURN_SERVICE_ERROR("Could not read characteristic", service, QLowEnergyService::CharacteristicReadError);

    auto readCompletedLambda = [charHandle, service]
            (IAsyncOperation<GattReadResult> const &op, winrt::AsyncStatus const status)
    {
        if (status == winrt::AsyncStatus::Canceled || status == winrt::AsyncStatus::Error) {
            qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle << "read operation failed.";
            service->setError(QLowEnergyService::CharacteristicReadError);
            return;
        }
        auto characteristicValue = SAFE(op.GetResults());
        if (!characteristicValue)
            RETURN_SERVICE_ERROR("Could not obtain result for characteristic", service, QLowEnergyService::CharacteristicReadError);

        const QByteArray value = byteArrayFromGattResult(characteristicValue);
        auto charData = service->characteristicList.value(charHandle);
        charData.value = value;
        service->characteristicList.insert(charHandle, charData);
        emit service->characteristicRead(QLowEnergyCharacteristic(service, charHandle), value);
        return;
    };

    if (!TRY(readOp.Completed(readCompletedLambda)))
        RETURN_SERVICE_ERROR("Could not register characteristic read callback", service, QLowEnergyService::CharacteristicReadError);
}

void QLowEnergyControllerPrivateWinRT::readDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service << charHandle << descHandle;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::DescriptorReadError);
        Q_UNIMPLEMENTED();
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "in characteristic" << charHandle
                             << "cannot be found in service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorReadError);
        return;
    }

    const auto charData = service->characteristicList.value(charHandle);

    auto characteristicCallback = [charHandle, descHandle, service, this](
            GattCharacteristic characteristic) {
        readDescriptorHelper(service, charHandle, descHandle, characteristic);
    };

    if (!getNativeCharacteristic(service->uuid, charData.uuid, characteristicCallback)) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native characteristic" << charData.uuid
                               << "from service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorReadError);
    }
}

void QLowEnergyControllerPrivateWinRT::readDescriptorHelper(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle,
        GattCharacteristic characteristic)
{
    // Get native descriptor
    const auto charData = service->characteristicList.value(charHandle);
    if (!charData.descriptorList.contains(descHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "cannot be found in characteristic"
                               << charHandle;
    }
    const auto descData = charData.descriptorList.value(descHandle);
    const QBluetoothUuid descUuid = descData.uuid;
    if (descUuid ==
            QBluetoothUuid(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration)) {

        auto readOp = SAFE(characteristic.ReadClientCharacteristicConfigurationDescriptorAsync());
        if (!readOp)
            RETURN_SERVICE_ERROR("Could not read client characteristic configuration", service, QLowEnergyService::DescriptorReadError);

        auto readCompletedLambda = [charHandle, descHandle, service]
                (IAsyncOperation<ClientCharConfigDescriptorResult> const &op, winrt::AsyncStatus const status)
        {
            if (status == winrt::AsyncStatus::Canceled || status == winrt::AsyncStatus::Error) {
                qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "read operation failed";
                service->setError(QLowEnergyService::DescriptorReadError);
                return;
            }

            auto iValue = SAFE(op.GetResults());
            if (!iValue)
                RETURN_SERVICE_ERROR("Could not obtain result for descriptor", service, QLowEnergyService::DescriptorReadError);
            GattClientCharacteristicConfigurationDescriptorValue value;
            if (!TRY(value = iValue.ClientCharacteristicConfigurationDescriptor()))
                RETURN_SERVICE_ERROR("Could not obtain value for descriptor", service, QLowEnergyService::DescriptorReadError);

            quint16 result = 0;
            bool correct = false;
            if (value & GattClientCharacteristicConfigurationDescriptorValue::Indicate) {
                result |= QLowEnergyCharacteristic::Indicate;
                correct = true;
            }
            if (value & GattClientCharacteristicConfigurationDescriptorValue::Notify) {
                result |= QLowEnergyCharacteristic::Notify;
                correct = true;
            }
            if (value == GattClientCharacteristicConfigurationDescriptorValue::None)
                correct = true;
            if (!correct) {
                qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle
                                       << "read operation failed. Obtained unexpected value.";
                service->setError(QLowEnergyService::DescriptorReadError);
                return;
            }
            QLowEnergyServicePrivate::DescData descData;
            descData.uuid = QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration;
            descData.value = QByteArray(2, Qt::Uninitialized);
            qToLittleEndian(result, descData.value.data());
            service->characteristicList[charHandle].descriptorList[descHandle] = descData;
            emit service->descriptorRead(QLowEnergyDescriptor(service, charHandle, descHandle),
                                         descData.value);
            return;
        };

        if (!TRY(readOp.Completed(readCompletedLambda)))
            RETURN_SERVICE_ERROR("Could not register descriptor read callback", service, QLowEnergyService::DescriptorReadError);

        return;
    }

    auto result = SAFE(await(characteristic.GetDescriptorsForUuidAsync(GUID(descData.uuid))));
    if (!result)
        RETURN_SERVICE_ERROR("Could not obtain descriptor for uuid", service, QLowEnergyService::DescriptorReadError);

    GattCommunicationStatus commStatus;
    if (!TRY(commStatus = result.Status()) || commStatus != GattCommunicationStatus::Success) {
        qErrnoWarning("Could not obtain list of descriptors");
        service->setError(QLowEnergyService::DescriptorReadError);
        return;
    }

    auto descriptors = SAFE(result.Descriptors());
    if (!descriptors)
        RETURN_SERVICE_ERROR("Could not obtain descriptor list", service, QLowEnergyService::DescriptorReadError);

    uint size;
    if (!TRY(size = descriptors.Size()))
        RETURN_SERVICE_ERROR("Could not obtain descriptor list size", service, QLowEnergyService::DescriptorReadError);

    if (size == 0) {
        qCWarning(QT_BT_WINDOWS) << "No descriptor with uuid" << descData.uuid << "was found.";
        service->setError(QLowEnergyService::DescriptorReadError);
        return;
    } else if (size > 1) {
        qCWarning(QT_BT_WINDOWS) << "There is more than 1 descriptor with uuid" << descData.uuid;
    }

    auto descriptor = SAFE(descriptors.GetAt(0));
    if (!descriptor)
        RETURN_SERVICE_ERROR("Could not obtain descriptor from list", service, QLowEnergyService::DescriptorReadError);

    auto readOp = SAFE(descriptor.ReadValueAsync(BluetoothCacheMode::Uncached));
    if (!readOp)
        RETURN_SERVICE_ERROR("Could not read descriptor value", service, QLowEnergyService::DescriptorReadError);

    auto readCompletedLambda = [charHandle, descHandle, descUuid, service]
            (IAsyncOperation<GattReadResult> const &op, winrt::AsyncStatus const status)
    {
        if (status == winrt::AsyncStatus::Canceled || status == winrt::AsyncStatus::Error) {
            qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "read operation failed";
            service->setError(QLowEnergyService::DescriptorReadError);
            return;
        }

        auto descriptorValue = SAFE(op.GetResults());
        if (!descriptorValue) {
            qCDebug(QT_BT_WINDOWS) << "Could not obtain result for descriptor" << descHandle;
            service->setError(QLowEnergyService::DescriptorReadError);
            return;
        }
        QLowEnergyServicePrivate::DescData descData;
        descData.uuid = descUuid;
        if (descData.uuid == QBluetoothUuid::DescriptorType::CharacteristicUserDescription)
            descData.value = byteArrayFromGattResult(descriptorValue, true);
        else
            descData.value = byteArrayFromGattResult(descriptorValue);
        service->characteristicList[charHandle].descriptorList[descHandle] = descData;
        emit service->descriptorRead(QLowEnergyDescriptor(service, charHandle, descHandle),
                                     descData.value);
    };
    if (!TRY(readOp.Completed(readCompletedLambda)))
        RETURN_SERVICE_ERROR("Could not register descriptor read callback", service, QLowEnergyService::DescriptorReadError);
}

void QLowEnergyControllerPrivateWinRT::writeCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QByteArray &newValue,
        QLowEnergyService::WriteMode mode)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service << charHandle << newValue << mode;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::CharacteristicWriteError);
        Q_UNIMPLEMENTED();
        return;
    }
    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle << "cannot be found in service"
                             << service->uuid;
        service->setError(QLowEnergyService::CharacteristicWriteError);
        return;
    }

    QLowEnergyServicePrivate::CharData charData = service->characteristicList.value(charHandle);
    const bool writeWithResponse = mode == QLowEnergyService::WriteWithResponse;
    if (!(charData.properties & (writeWithResponse ? QLowEnergyCharacteristic::Write
             : QLowEnergyCharacteristic::WriteNoResponse)))
        qCDebug(QT_BT_WINDOWS) << "Write flag is not set for characteristic" << charHandle;

    auto characteristicCallback = [charHandle, service, newValue, writeWithResponse, this](
            GattCharacteristic characteristic) {
        writeCharacteristicHelper(service, charHandle, newValue, writeWithResponse,
                                  characteristic);
    };

    if (!getNativeCharacteristic(service->uuid, charData.uuid, characteristicCallback)) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native characteristic" << charData.uuid
                               << "from service" << service->uuid;
        service->setError(QLowEnergyService::CharacteristicWriteError);
    }
}

void QLowEnergyControllerPrivateWinRT::writeCharacteristicHelper(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle, const QByteArray &newValue,
        bool writeWithResponse, GattCharacteristic characteristic)
{
    const quint32 length = quint32(newValue.length());
    Buffer buffer = nullptr;
    if (!TRY(buffer = Buffer(length)))
        RETURN_SERVICE_ERROR("Could not create buffer", service, QLowEnergyService::CharacteristicWriteError);

    byte *bytes;
    if (!TRY(bytes = buffer.data()))
        RETURN_SERVICE_ERROR("Could not set buffer", service, QLowEnergyService::CharacteristicWriteError);

    memcpy(bytes, newValue, length);
    if (!TRY(buffer.Length(length)))
        RETURN_SERVICE_ERROR("Could not set buffer length", service, QLowEnergyService::CharacteristicWriteError);

    GattWriteOption option = writeWithResponse ? GattWriteOption::WriteWithResponse
                                               : GattWriteOption::WriteWithoutResponse;
    auto writeOp = SAFE(characteristic.WriteValueAsync(buffer, option));
    if (!writeOp)
        RETURN_SERVICE_ERROR("Could not write characteristic", service, QLowEnergyService::CharacteristicWriteError);

    const auto charData = service->characteristicList.value(charHandle);
    QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
    auto writeCompletedLambda =
            [charData, charHandle, newValue, service, writeWithResponse, thisPtr]
                (IAsyncOperation<GattCommunicationStatus> const &op, winrt::AsyncStatus const status)
    {
        if (status == winrt::AsyncStatus::Canceled || status == winrt::AsyncStatus::Error) {
            qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle
                                   << "write operation failed (async status)";
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return;
        }

        GattCommunicationStatus result;
        auto hr = HR(result = op.GetResults());
        if (hr == E_BLUETOOTH_ATT_INVALID_ATTRIBUTE_VALUE_LENGTH) {
            qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle
                                   << "write operation was tried with invalid value length";
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return;
        } else if (FAILED(hr)) {
            RETURN_SERVICE_ERROR("Could not obtain characteristic write result", service, QLowEnergyService::CharacteristicWriteError);
        }

        if (result != GattCommunicationStatus::Success) {
            qCDebug(QT_BT_WINDOWS) << "Characteristic" << charHandle
                                   << "write operation failed (communication status)";
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return;
        }
        // only update cache when property is readable. Otherwise it remains
        // empty.
        if (thisPtr && charData.properties & QLowEnergyCharacteristic::Read)
            thisPtr->updateValueOfCharacteristic(charHandle, newValue, false);
        if (writeWithResponse) {
            emit service->characteristicWritten(QLowEnergyCharacteristic(service, charHandle),
                                                newValue);
        }
    };

    if (!TRY(writeOp.Completed(writeCompletedLambda)))
        RETURN_SERVICE_ERROR("Could not register characteristic write callback", service, QLowEnergyService::CharacteristicWriteError);
}

void QLowEnergyControllerPrivateWinRT::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle,
        const QByteArray &newValue)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << service << charHandle << descHandle << newValue;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::DescriptorWriteError);
        Q_UNIMPLEMENTED();
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "in characteristic" << charHandle
                             << "could not be found in service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorWriteError);
        return;
    }

    const auto charData = service->characteristicList.value(charHandle);

    auto characteristicCallback = [descHandle, charHandle, service, newValue, this](
            GattCharacteristic characteristic) {
        writeDescriptorHelper(service, charHandle, descHandle, newValue, characteristic);
    };

    if (!getNativeCharacteristic(service->uuid, charData.uuid, characteristicCallback)) {
        qCDebug(QT_BT_WINDOWS) << "Could not obtain native characteristic" << charData.uuid
                               << "from service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorWriteError);
    }
}

void QLowEnergyControllerPrivateWinRT::writeDescriptorHelper(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle,
        const QByteArray &newValue,
        GattCharacteristic characteristic)
{
    // Get native descriptor
    const auto charData = service->characteristicList.value(charHandle);
    if (!charData.descriptorList.contains(descHandle)) {
        qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle
                               << "could not be found in Characteristic" << charHandle;
    }

    QLowEnergyServicePrivate::DescData descData = charData.descriptorList.value(descHandle);
    if (descData.uuid ==
            QBluetoothUuid(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration)) {
        GattClientCharacteristicConfigurationDescriptorValue value;
        quint16 intValue = qFromLittleEndian<quint16>(newValue);
        if ((intValue & GattClientCharacteristicConfigurationDescriptorValue::Indicate)
                && (intValue & GattClientCharacteristicConfigurationDescriptorValue::Notify)) {
            qCWarning(QT_BT_WINDOWS) << "Setting both Indicate and Notify "
                                        "is not supported on WinRT";
            value = GattClientCharacteristicConfigurationDescriptorValue(
                    (GattClientCharacteristicConfigurationDescriptorValue::Indicate
                     | GattClientCharacteristicConfigurationDescriptorValue::Notify));
        } else if (intValue & GattClientCharacteristicConfigurationDescriptorValue::Indicate) {
            value = GattClientCharacteristicConfigurationDescriptorValue::Indicate;
        } else if (intValue & GattClientCharacteristicConfigurationDescriptorValue::Notify) {
            value = GattClientCharacteristicConfigurationDescriptorValue::Notify;
        } else if (intValue == 0) {
            value = GattClientCharacteristicConfigurationDescriptorValue::None;
        } else {
            qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle
                                   << "write operation failed: Invalid value";
            service->setError(QLowEnergyService::DescriptorWriteError);
            return;
        }

        auto writeOp = SAFE(characteristic.WriteClientCharacteristicConfigurationDescriptorAsync(value));
        if (!writeOp)
            RETURN_SERVICE_ERROR("Could not write client characteristic configuration", service, QLowEnergyService::DescriptorWriteError);

        QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
        auto writeCompletedLambda = [charHandle, descHandle, newValue, service, thisPtr]
                (IAsyncOperation<GattCommunicationStatus> const &op, winrt::AsyncStatus const status)
        {
            if (status == winrt::AsyncStatus::Canceled || status == winrt::AsyncStatus::Error) {
                qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "write operation failed";
                service->setError(QLowEnergyService::DescriptorWriteError);
                return;
            }

            GattCommunicationStatus result;
            if (!TRY(result = op.GetResults()))
                RETURN_SERVICE_ERROR("Could not obtain result for descriptor", service, QLowEnergyService::DescriptorWriteError);

            if (result != GattCommunicationStatus::Success) {
                qCWarning(QT_BT_WINDOWS) << "Descriptor" << descHandle << "write operation failed";
                service->setError(QLowEnergyService::DescriptorWriteError);
                return;
            }
            if (thisPtr)
                thisPtr->updateValueOfDescriptor(charHandle, descHandle, newValue, false);
            emit service->descriptorWritten(QLowEnergyDescriptor(service, charHandle, descHandle),
                                            newValue);
        };

        if (!TRY(writeOp.Completed(writeCompletedLambda)))
            RETURN_SERVICE_ERROR("Could not register descriptor write callback", service, QLowEnergyService::DescriptorWriteError);

        return;
    }

    auto result = SAFE(await(characteristic.GetDescriptorsForUuidAsync(GUID(descData.uuid))));
    if (!result)
        RETURN_SERVICE_ERROR("Could not obtain descriptor from Uuid", service, QLowEnergyService::DescriptorWriteError);

    GattCommunicationStatus commStatus;
    if (!TRY(commStatus = result.Status()) || commStatus != GattCommunicationStatus::Success) {
        qCWarning(QT_BT_WINDOWS) << "Descriptor operation failed";
        service->setError(QLowEnergyService::DescriptorWriteError);
        return;
    }

    auto descriptors = SAFE(result.Descriptors());
    if (!descriptors)
        RETURN_SERVICE_ERROR("Could not obtain list of descriptors", service, QLowEnergyService::DescriptorWriteError);

    uint size;
    if (!TRY(size = descriptors.Size()))
        RETURN_SERVICE_ERROR("Could not obtain list of descriptors' size", service, QLowEnergyService::DescriptorWriteError);

    if (size == 0) {
        qCWarning(QT_BT_WINDOWS) << "No descriptor with uuid" << descData.uuid << "was found.";
        return;
    } else if (size > 1) {
        qCWarning(QT_BT_WINDOWS) << "There is more than 1 descriptor with uuid" << descData.uuid;
    }

    auto descriptor = SAFE(descriptors.GetAt(0));
    if (!descriptor)
        RETURN_SERVICE_ERROR("Could not obtain descriptor", service, QLowEnergyService::DescriptorWriteError);

    const quint32 length = quint32(newValue.length());
    Buffer buffer = nullptr;
    if (!TRY(buffer = Buffer(length)))
        RETURN_SERVICE_ERROR("Could not create buffer", service, QLowEnergyService::CharacteristicWriteError);

    byte *bytes;
    if (!TRY(bytes = buffer.data()))
        RETURN_SERVICE_ERROR("Could not set buffer", service, QLowEnergyService::CharacteristicWriteError);

    memcpy(bytes, newValue, length);
    if (!TRY(buffer.Length(length)))
        RETURN_SERVICE_ERROR("Could not set buffer length", service, QLowEnergyService::CharacteristicWriteError);

    auto writeOp = SAFE(descriptor.WriteValueAsync(buffer));
    if (!writeOp)
        RETURN_SERVICE_ERROR("Could not write descriptor value", service, QLowEnergyService::CharacteristicWriteError);

    QPointer<QLowEnergyControllerPrivateWinRT> thisPtr(this);
    auto writeCompletedLambda = [charHandle, descHandle, newValue, service, thisPtr]
            (IAsyncOperation<GattCommunicationStatus> const &op, winrt::AsyncStatus const status)
    {
        if (status == winrt::AsyncStatus::Canceled || status == winrt::AsyncStatus::Error) {
            qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "write operation failed";
            service->setError(QLowEnergyService::DescriptorWriteError);
            return;
        }

        GattCommunicationStatus result;
        if (!TRY(result = op.GetResults()))
            RETURN_SERVICE_ERROR("Could not obtain result for descriptor", service, QLowEnergyService::DescriptorWriteError);

        if (result != GattCommunicationStatus::Success) {
            qCDebug(QT_BT_WINDOWS) << "Descriptor" << descHandle << "write operation failed";
            service->setError(QLowEnergyService::DescriptorWriteError);
            return;
        }
        if (thisPtr)
            thisPtr->updateValueOfDescriptor(charHandle, descHandle, newValue, false);
        emit service->descriptorWritten(QLowEnergyDescriptor(service, charHandle, descHandle),
                                        newValue);
    };

    if (!TRY(writeOp.Completed(writeCompletedLambda)))
        RETURN_SERVICE_ERROR("Could not register descriptor write callback", service, QLowEnergyService::DescriptorWriteError);

}

void QLowEnergyControllerPrivateWinRT::addToGenericAttributeList(const QLowEnergyServiceData &,
                                                                    QLowEnergyHandle)
{
    Q_UNIMPLEMENTED();
}

int QLowEnergyControllerPrivateWinRT::mtu() const
{
    uint16_t mtu = 23;
    if (!mGattSession) {
        qCDebug(QT_BT_WINDOWS) << "mtu queried before GattSession available. Using default mtu.";
        return mtu;
    }

    if (!TRY(mtu = mGattSession.MaxPduSize()))
        RETURN_FALSE("could not obtain MTU size");

    qCDebug(QT_BT_WINDOWS) << "mtu determined to be" << mtu;
    return mtu;
}

void QLowEnergyControllerPrivateWinRT::handleCharacteristicChanged(
        quint16 charHandle, const QByteArray &data)
{
    qCDebug(QT_BT_WINDOWS) << __FUNCTION__ << charHandle << data;
    qCDebug(QT_BT_WINDOWS_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(charHandle);
    if (service.isNull())
        return;

    qCDebug(QT_BT_WINDOWS) << "Characteristic change notification" << service->uuid
                           << charHandle << data.toHex();

    QLowEnergyCharacteristic characteristic = characteristicForHandle(charHandle);
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_WINDOWS) << "characteristicChanged: Cannot find characteristic";
        return;
    }

    // only update cache when property is readable. Otherwise it remains
    // empty.
    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(characteristic.attributeHandle(),
                                data, false);
    emit service->characteristicChanged(characteristic, data);
}

void QLowEnergyControllerPrivateWinRT::handleServiceHandlerError(const QString &error)
{
    if (state != QLowEnergyController::DiscoveringState)
        return;

    qCWarning(QT_BT_WINDOWS) << "Error while discovering services:" << error;
    setState(QLowEnergyController::UnconnectedState);
    setError(QLowEnergyController::ConnectionError);
}

void QLowEnergyControllerPrivateWinRT::handleConnectionError(const char *logMessage)
{
    qCWarning(QT_BT_WINDOWS) << logMessage;
    setError(QLowEnergyController::ConnectionError);
    setState(QLowEnergyController::UnconnectedState);
    unregisterFromStatusChanges();
    unregisterFromMtuChanges();
}

QT_END_NAMESPACE

#include "qlowenergycontroller_winrt.moc"
