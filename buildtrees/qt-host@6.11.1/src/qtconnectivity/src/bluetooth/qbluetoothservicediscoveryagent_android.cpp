// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothservicediscoveryagent_p.h"
#include "qbluetoothsocket_android_p.h"
#include "android/servicediscoverybroadcastreceiver_p.h"
#include "android/localdevicebroadcastreceiver_p.h"
#include "android/androidutils_p.h"
#include "android/jni_android_p.h"

#include <QCoreApplication>
#include <QtCore/qcoreapplication.h>
#include <QtCore/QLoggingCategory>
#include <QtCore/QTimer>
#include <QtCore/QJniEnvironment>
#include <QtBluetooth/QBluetoothHostInfo>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothServiceDiscoveryAgent>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_BT_ANDROID)

static constexpr auto uuidFetchTimeLimit = std::chrono::seconds{4};

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
        QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &deviceAdapter)
    : error(QBluetoothServiceDiscoveryAgent::NoError),
      m_deviceAdapterAddress(deviceAdapter),
      state(Inactive),
      mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery),
      singleDevice(false),
      q_ptr(qp)

{
    // If a specific adapter address is requested we need to check it matches
    // the current local adapter. If it does not match we emit
    // InvalidBluetoothAdapterError when calling start()

    bool createAdapter = true;
    if (!deviceAdapter.isNull()) {
        const QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
        if (devices.isEmpty()) {
            createAdapter = false;
        } else {
            auto match = [deviceAdapter](const QBluetoothHostInfo& info) {
                return info.address() == deviceAdapter;
            };

            auto result = std::find_if(devices.begin(), devices.end(), match);
            if (result == devices.end())
                createAdapter = false;
        }
    }

    /*
      We assume that the current local adapter has been passed.
      The logic below must change once there is more than one adapter.
    */

    if (createAdapter)
        btAdapter = getDefaultBluetoothAdapter();

    if (!btAdapter.isValid())
        qCWarning(QT_BT_ANDROID) << "Platform does not support Bluetooth";

    qRegisterMetaType<QList<QBluetoothUuid> >();
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
    if (receiver) {
        receiver->unregisterReceiver();
        delete receiver;
    }
    if (localDeviceReceiver) {
        localDeviceReceiver->unregisterReceiver();
        delete localDeviceReceiver;
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &address)
{
    Q_Q(QBluetoothServiceDiscoveryAgent);

    if (!ensureAndroidPermission(QBluetoothPermission::Access)) {
        qCWarning(QT_BT_ANDROID) << "Service discovery start() failed due to missing permissions";
        error = QBluetoothServiceDiscoveryAgent::MissingPermissionsError;
        errorString = QBluetoothServiceDiscoveryAgent::tr(
                "Failed to start service discovery due to missing permissions.");
        emit q->errorOccurred(error);
        _q_serviceDiscoveryFinished();
        return;
    }

    if (!btAdapter.isValid()) {
        if (m_deviceAdapterAddress.isNull()) {
            error = QBluetoothServiceDiscoveryAgent::UnknownError;
            errorString = QBluetoothServiceDiscoveryAgent::tr("Platform does not support Bluetooth");
        } else {
            // specific adapter was requested which does not match the locally
            // existing adapter
            error = QBluetoothServiceDiscoveryAgent::InvalidBluetoothAdapterError;
            errorString = QBluetoothServiceDiscoveryAgent::tr("Invalid Bluetooth adapter address");
        }

        //abort any outstanding discoveries
        discoveredDevices.clear();
        emit q->errorOccurred(error);
        _q_serviceDiscoveryFinished();

        return;
    }

    QJniObject inputString = QJniObject::fromString(address.toString());
    QJniObject remoteDevice =
            btAdapter.callMethod<QtJniTypes::BluetoothDevice>("getRemoteDevice",
                                                              inputString.object<jstring>());
    if (!remoteDevice.isValid()) {

        //if it was only device then its error -> otherwise go to next device
        if (singleDevice) {
            error = QBluetoothServiceDiscoveryAgent::InputOutputError;
            errorString = QBluetoothServiceDiscoveryAgent::tr("Cannot create Android BluetoothDevice");

            qCWarning(QT_BT_ANDROID) << "Cannot start SDP for" << discoveredDevices.at(0).name()
                                     << "(" << address.toString() << ")";
            emit q->errorOccurred(error);
        }
        _q_serviceDiscoveryFinished();
        return;
    }


    if (mode == QBluetoothServiceDiscoveryAgent::MinimalDiscovery) {
        qCDebug(QT_BT_ANDROID) << "Minimal discovery on (" << discoveredDevices.at(0).name()
                               << ")" << address.toString() ;

        //Minimal discovery uses BluetoothDevice.getUuids()
        const QJniArray parcelUuidArray =
                remoteDevice.callMethod<QtJniTypes::ParcelUuid[]>("getUuids");

        if (!parcelUuidArray.isValid()) {
            if (singleDevice) {
                error = QBluetoothServiceDiscoveryAgent::InputOutputError;
                errorString = QBluetoothServiceDiscoveryAgent::tr("Cannot obtain service uuids");
                emit q->errorOccurred(error);
            }
            qCWarning(QT_BT_ANDROID) << "Cannot retrieve SDP UUIDs for" << discoveredDevices.at(0).name()
                                     << "(" << address.toString() << ")";
            _q_serviceDiscoveryFinished();
            return;
        }

        const QList<QBluetoothUuid> results = ServiceDiscoveryBroadcastReceiver::convertParcelableArray(parcelUuidArray);
        populateDiscoveredServices(discoveredDevices.at(0), results);

        _q_serviceDiscoveryFinished();
    } else {
        qCDebug(QT_BT_ANDROID) << "Full discovery on (" << discoveredDevices.at(0).name()
                               << ")" << address.toString();

        //Full discovery uses BluetoothDevice.fetchUuidsWithSdp()
        if (!receiver) {
            receiver = new ServiceDiscoveryBroadcastReceiver();
            QObject::connect(receiver, &ServiceDiscoveryBroadcastReceiver::uuidFetchFinished,
                             q, [this](const QBluetoothAddress &address, const QList<QBluetoothUuid>& uuids) {
                this->_q_processFetchedUuids(address, uuids);
            });
        }

        if (!localDeviceReceiver) {
            localDeviceReceiver = new LocalDeviceBroadcastReceiver();
            QObject::connect(localDeviceReceiver, &LocalDeviceBroadcastReceiver::hostModeStateChanged,
                             q, [this](QBluetoothLocalDevice::HostMode state){
                this->_q_hostModeStateChanged(state);
            });
        }

        jboolean result = remoteDevice.callMethod<jboolean>("fetchUuidsWithSdp");
        if (!result) {
            //kill receiver to limit load of signals
            receiver->unregisterReceiver();
            receiver->deleteLater();
            receiver = nullptr;
            qCWarning(QT_BT_ANDROID) << "Cannot start dynamic fetch.";
            _q_serviceDiscoveryFinished();
        }
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    sdpCache.clear();
    discoveredDevices.clear();

    //kill receiver to limit load of signals
    if (receiver) {
        receiver->unregisterReceiver();
        receiver->deleteLater();
        receiver = nullptr;
    }

    Q_Q(QBluetoothServiceDiscoveryAgent);
    emit q->canceled();

}

void QBluetoothServiceDiscoveryAgentPrivate::_q_processFetchedUuids(
    const QBluetoothAddress &address, const QList<QBluetoothUuid> &uuids)
{
    //don't leave more data through if we are not interested anymore
    if (discoveredDevices.isEmpty())
        return;

    //could not find any service for the current address/device -> go to next one
    if (address.isNull() || uuids.isEmpty()) {
        if (discoveredDevices.size() == 1) {
            Q_Q(QBluetoothServiceDiscoveryAgent);
            QTimer::singleShot(uuidFetchTimeLimit, q, [this]() {
                this->_q_fetchUuidsTimeout();
            }); // will also call _q_serviceDiscoveryFinished()
        } else {
            _q_serviceDiscoveryFinished();
        }
        return;
    }

    if (QT_BT_ANDROID().isDebugEnabled()) {
        qCDebug(QT_BT_ANDROID) << "Found UUID for" << address.toString()
                               << "\ncount: " << uuids.size();

        QString result;
        for (const QBluetoothUuid &uuid : uuids)
            result += uuid.toString() + QLatin1String("**");
        qCDebug(QT_BT_ANDROID) << result;
    }

    /* In general there may be up-to two uuid events per device.
     * We'll wait for the second event to arrive before we process the UUIDs.
     * We utilize a timeout to catch cases when the second
     * event doesn't arrive at all.
     * Generally we assume that the second uuid event carries the most up-to-date
     * set of uuids and discard the first events results.
    */

    if (sdpCache.contains(address)) {
        //second event
        QPair<QBluetoothDeviceInfo,QList<QBluetoothUuid> > pair = sdpCache.take(address);

        //prefer second uuid set over first
        populateDiscoveredServices(pair.first, uuids);

        if (discoveredDevices.size() == 1 && sdpCache.isEmpty()) {
            //last regular uuid data set from OS -> we finish here
            _q_serviceDiscoveryFinished();
        }
    } else {
        //first event
        QPair<QBluetoothDeviceInfo,QList<QBluetoothUuid> > pair;
        pair.first = discoveredDevices.at(0);
        pair.second = uuids;

        if (pair.first.address() != address)
            return;

        sdpCache.insert(address, pair);

        //the discovery on the last device cannot immediately finish
        //we have to grant the timeout delay to allow potential second event to arrive
        if (discoveredDevices.size() == 1) {
            Q_Q(QBluetoothServiceDiscoveryAgent);
            QTimer::singleShot(uuidFetchTimeLimit, q, [this]() {
                this->_q_fetchUuidsTimeout();
            });
            return;
        }

        _q_serviceDiscoveryFinished();
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::populateDiscoveredServices(const QBluetoothDeviceInfo &remoteDevice, const QList<QBluetoothUuid> &uuids)
{
    /* Android doesn't provide decent SDP data. A flat list of uuids is all we get.
     *
     * The following approach is chosen:
     * - If we see an SPP service class and we see
     *   one or more custom uuids we match them up. Such services will always
     *   be SPP services. There is the chance that a custom uuid is eronously
     *   mapped as being an SPP service. In addition, the SPP uuid will be mapped as
     *   standalone SPP service.
     * - If we see a custom uuid but no SPP uuid then we return
     *   BluetoothServiceInfo instance with just a serviceUuid (no service class set)
     * - If we don't find any custom uuid but the SPP uuid, we return a
     *   BluetoothServiceInfo instance where classId and serviceUuid() are set to SPP.
     * - Any other service uuid will stand on its own.
     * */

    Q_Q(QBluetoothServiceDiscoveryAgent);

    //find SPP and custom uuid
    bool haveSppClass = false;
    QVarLengthArray<qsizetype> customUuids;

    for (qsizetype i = 0; i < uuids.size(); ++i) {
        const QBluetoothUuid uuid = uuids.at(i);

        if (uuid.isNull())
            continue;

        //check for SPP protocol
        haveSppClass |= uuid == QBluetoothUuid::ServiceClassUuid::SerialPort;

        //check for custom uuid
        if (uuid.minimumSize() == 16)
            customUuids.append(i);
    }

    auto rfcommProtocolDescriptorList = []() -> QBluetoothServiceInfo::Sequence {
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::Rfcomm))
                     << QVariant::fromValue(0);
            return protocol;
    };

    auto sppProfileDescriptorList = []() -> QBluetoothServiceInfo::Sequence {
        QBluetoothServiceInfo::Sequence profileSequence;
        QBluetoothServiceInfo::Sequence classId;
        classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
        classId << QVariant::fromValue(quint16(0x100));
        profileSequence.append(QVariant::fromValue(classId));
        return profileSequence;
    };

    for (qsizetype i = 0; i < uuids.size(); ++i) {
        const QBluetoothUuid &uuid = uuids.at(i);
        if (uuid.isNull())
            continue;

        QBluetoothServiceInfo serviceInfo;
        serviceInfo.setDevice(remoteDevice);

        QBluetoothServiceInfo::Sequence protocolDescriptorList;
        {
            QBluetoothServiceInfo::Sequence protocol;
            protocol << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ProtocolUuid::L2cap));
            protocolDescriptorList.append(QVariant::fromValue(protocol));
        }

        if (customUuids.contains(i) && haveSppClass) {
            //we have a custom uuid of service class type SPP

            //set rfcomm protocol
            protocolDescriptorList.append(QVariant::fromValue(rfcommProtocolDescriptorList()));

            //set SPP profile descriptor list
            serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                                     sppProfileDescriptorList());

            QBluetoothServiceInfo::Sequence classId;
            //set SPP service class uuid
            classId << QVariant::fromValue(uuid);
            classId << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::SerialPort));
            serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);

            serviceInfo.setServiceName(QBluetoothServiceDiscoveryAgent::tr("Serial Port Profile"));
            serviceInfo.setServiceUuid(uuid);
        } else if (uuid == QBluetoothUuid{QBluetoothUuid::ServiceClassUuid::SerialPort}) {
            //set rfcomm protocol
            protocolDescriptorList.append(QVariant::fromValue(rfcommProtocolDescriptorList()));

            //set SPP profile descriptor list
            serviceInfo.setAttribute(QBluetoothServiceInfo::BluetoothProfileDescriptorList,
                                     sppProfileDescriptorList());

            //also we need to set the custom uuid to the SPP uuid
            //otherwise QBluetoothSocket::connectToService() would fail due to a missing service uuid
            serviceInfo.setServiceUuid(uuid);
        } else if (customUuids.contains(i)) {
            //custom uuid but no serial port
            serviceInfo.setServiceUuid(uuid);
        }

        serviceInfo.setAttribute(QBluetoothServiceInfo::ProtocolDescriptorList, protocolDescriptorList);
        QBluetoothServiceInfo::Sequence publicBrowse;
        publicBrowse << QVariant::fromValue(QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::PublicBrowseGroup));
        serviceInfo.setAttribute(QBluetoothServiceInfo::BrowseGroupList, publicBrowse);

        if (!customUuids.contains(i)) {
            //if we don't have custom uuid use it as class id as well
            QBluetoothServiceInfo::Sequence classId;
            classId << QVariant::fromValue(uuid);
            serviceInfo.setAttribute(QBluetoothServiceInfo::ServiceClassIds, classId);
            auto clsId = QBluetoothUuid::ServiceClassUuid(uuid.toUInt16());
            serviceInfo.setServiceName(QBluetoothUuid::serviceClassToString(clsId));
        }

        //Check if the service is in the uuidFilter
        if (!uuidFilter.isEmpty()) {
            bool match = uuidFilter.contains(serviceInfo.serviceUuid());
            match |= uuidFilter.contains(QBluetoothSocketPrivateAndroid::reverseUuid(serviceInfo.serviceUuid()));
            for (const auto &uuid : std::as_const(uuidFilter)) {
                match |= serviceInfo.serviceClassUuids().contains(uuid);
                match |= serviceInfo.serviceClassUuids().contains(QBluetoothSocketPrivateAndroid::reverseUuid(uuid));
            }

            if (!match)
                continue;
        }

        //don't include the service if we already discovered it before
        if (!isDuplicatedService(serviceInfo)) {
            discoveredServices << serviceInfo;
            // Use queued connection to allow us finish the service discovery reporting;
            // the application might call stop() when it has detected the service-of-interest,
            // which in turn can cause the use of already released resources
            QMetaObject::invokeMethod(q, "serviceDiscovered", Qt::QueuedConnection,
                                      Q_ARG(QBluetoothServiceInfo, serviceInfo));
        }
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_fetchUuidsTimeout()
{
    // In practice if device list is empty, discovery has been stopped or bluetooth is offline
    if (discoveredDevices.isEmpty())
        return;

    // Process remaining services in the cache (these didn't get a second UUID event)
    if (!sdpCache.isEmpty()) {
        QPair<QBluetoothDeviceInfo,QList<QBluetoothUuid> > pair;
        const QList<QBluetoothAddress> keys = sdpCache.keys();
        for (const QBluetoothAddress &key : keys) {
            pair = sdpCache.take(key);
            populateDiscoveredServices(pair.first, pair.second);
        }
    }

    Q_ASSERT(sdpCache.isEmpty());

    //kill receiver to limit load of signals
    if (receiver) {
        receiver->unregisterReceiver();
        receiver->deleteLater();
        receiver = nullptr;
    }
    _q_serviceDiscoveryFinished();
}

void QBluetoothServiceDiscoveryAgentPrivate::_q_hostModeStateChanged(QBluetoothLocalDevice::HostMode state)
{
    if (discoveryState() == QBluetoothServiceDiscoveryAgentPrivate::ServiceDiscovery &&
            state == QBluetoothLocalDevice::HostPoweredOff ) {

        discoveredDevices.clear();
        sdpCache.clear();
        error = QBluetoothServiceDiscoveryAgent::PoweredOffError;
        errorString = QBluetoothServiceDiscoveryAgent::tr("Device is powered off");

        //kill receiver to limit load of signals
        if (receiver) {
            receiver->unregisterReceiver();
            receiver->deleteLater();
            receiver = nullptr;
        }

        Q_Q(QBluetoothServiceDiscoveryAgent);
        emit q->errorOccurred(error);
        _q_serviceDiscoveryFinished();
    }
}

QT_END_NAMESPACE
