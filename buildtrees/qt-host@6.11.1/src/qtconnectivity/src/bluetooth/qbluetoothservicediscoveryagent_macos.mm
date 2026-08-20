// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothservicediscoveryagent_p.h"
#include "qbluetoothservicediscoveryagent.h"
#include "qbluetoothdevicediscoveryagent.h"
#include "qbluetoothlocaldevice.h"
#include "darwin/btsdpinquiry_p.h"
#include "qbluetoothhostinfo.h"
#include "darwin/btutility_p.h"
#include "darwin/uistrings_p.h"

#include <QtCore/qoperatingsystemversion.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qstring.h>
#include <QtCore/qglobal.h>
#include <QtCore/qdebug.h>
#include <QtCore/qlist.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

QT_BEGIN_NAMESPACE

namespace {

using DarwinBluetooth::RetainPolicy;

}

QBluetoothServiceDiscoveryAgentPrivate::QBluetoothServiceDiscoveryAgentPrivate(
    QBluetoothServiceDiscoveryAgent *qp, const QBluetoothAddress &localAddress) :

    error(QBluetoothServiceDiscoveryAgent::NoError),
    m_deviceAdapterAddress(localAddress),
    state(Inactive),
    mode(QBluetoothServiceDiscoveryAgent::MinimalDiscovery),
    singleDevice(false),
    q_ptr(qp)

{
    Q_ASSERT(q_ptr);
    serviceInquiry.reset([[DarwinBTSDPInquiry alloc] initWithDelegate:this], RetainPolicy::noInitialRetain);
}

QBluetoothServiceDiscoveryAgentPrivate::~QBluetoothServiceDiscoveryAgentPrivate()
{
}

void QBluetoothServiceDiscoveryAgentPrivate::start(const QBluetoothAddress &deviceAddress)
{
    QT_BT_MAC_AUTORELEASEPOOL;

    if (deviceAddress.isNull()) {
        // This can happen: LE scan works with CoreBluetooth, but CBPeripherals
        // do not expose hardware addresses.
        // Pop the current QBluetoothDeviceInfo and decide what to do next.
        return _q_serviceDiscoveryFinished();
    }

    // Autoreleased object.
    IOBluetoothHostController *const hc = [IOBluetoothHostController defaultController];
    if (![hc powerState]) {
        discoveredDevices.clear();
        if (singleDevice) {
            error = QBluetoothServiceDiscoveryAgent::PoweredOffError;
            errorString = QCoreApplication::translate(SERVICE_DISCOVERY, SD_LOCAL_DEV_OFF);
            emit q_ptr->errorOccurred(error);
        }

        return _q_serviceDiscoveryFinished();
    }

    if (DiscoveryMode() == QBluetoothServiceDiscoveryAgent::MinimalDiscovery) {
        performMinimalServiceDiscovery(deviceAddress);
    } else {
        IOReturn result = kIOReturnSuccess;
        auto nativeInquiry = serviceInquiry.getAs<DarwinBTSDPInquiry>();
        if (uuidFilter.size())
            result = [nativeInquiry performSDPQueryWithDevice:deviceAddress filters:uuidFilter];
        else
            result = [nativeInquiry performSDPQueryWithDevice:deviceAddress];

        if (result != kIOReturnSuccess) {
            // Failed immediately to perform an SDP inquiry on IOBluetoothDevice:
            SDPInquiryError(nil, result);
        }
    }
}

void QBluetoothServiceDiscoveryAgentPrivate::stop()
{
    Q_ASSERT_X(q_ptr, Q_FUNC_INFO, "invalid q_ptr (null)");

    discoveredDevices.clear();

    // "Stops" immediately.
    [serviceInquiry.getAs<DarwinBTSDPInquiry>() stopSDPQuery];

    emit q_ptr->canceled();
}

void QBluetoothServiceDiscoveryAgentPrivate::SDPInquiryFinished(void *generic)
{
    auto device = static_cast<IOBluetoothDevice *>(generic);
    Q_ASSERT_X(device, Q_FUNC_INFO, "invalid IOBluetoothDevice (nil)");

    if (state == Inactive)
        return;

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const records = device.services;
    qCDebug(QT_BT_DARWIN) << "SDP finished for device" << [device nameOrAddress]
                          << ", services found:" << [records count];
    for (IOBluetoothSDPServiceRecord *record in records) {
        QBluetoothServiceInfo serviceInfo;
        Q_ASSERT_X(discoveredDevices.size() >= 1, Q_FUNC_INFO, "invalid number of devices");

        qCDebug(QT_BT_DARWIN) << "Processing service" << [record getServiceName];
        serviceInfo.setDevice(discoveredDevices.at(0));
        DarwinBluetooth::extract_service_record(record, serviceInfo);

        if (!serviceInfo.isValid()) {
            qCDebug(QT_BT_DARWIN) << "Discarding invalid service";
            continue;
        }

        if (QOperatingSystemVersion::current() > QOperatingSystemVersion::MacOSBigSur
            && uuidFilter.size()) {
            const auto &serviceId = serviceInfo.serviceUuid();
            bool match = !serviceId.isNull() && uuidFilter.contains(serviceId);
            if (!match) {
                const auto &classUuids = serviceInfo.serviceClassUuids();
                for (const auto &uuid : classUuids) {
                    if (uuidFilter.contains(uuid)) {
                        match = true;
                        break;
                    }
                }
                if (!match)
                    continue;
            }
        }


        if (!isDuplicatedService(serviceInfo)) {
            discoveredServices.append(serviceInfo);
            emit q_ptr->serviceDiscovered(serviceInfo);
            // Here a user code can ... interrupt us by calling
            // stop. Check this.
            if (state == Inactive)
                break;
        }
    }

    _q_serviceDiscoveryFinished();
}

void QBluetoothServiceDiscoveryAgentPrivate::SDPInquiryError(void *device, IOReturn errorCode)
{
    Q_UNUSED(device);

    qCWarning(QT_BT_DARWIN) << "inquiry failed with IOKit code:" << int(errorCode);

    discoveredDevices.clear();
    // TODO: find a better mapping from IOReturn to QBluetoothServiceDiscoveryAgent::Error.
    if (singleDevice) {
        error = QBluetoothServiceDiscoveryAgent::UnknownError;
        errorString = QCoreApplication::translate(DEV_DISCOVERY, DD_UNKNOWN_ERROR);
        emit q_ptr->errorOccurred(error);
    }

    _q_serviceDiscoveryFinished();
}

void QBluetoothServiceDiscoveryAgentPrivate::performMinimalServiceDiscovery(const QBluetoothAddress &deviceAddress)
{
    Q_ASSERT_X(!deviceAddress.isNull(), Q_FUNC_INFO, "invalid device address");

    QT_BT_MAC_AUTORELEASEPOOL;

    const BluetoothDeviceAddress iobtAddress = DarwinBluetooth::iobluetooth_address(deviceAddress);
    IOBluetoothDevice *const device = [IOBluetoothDevice deviceWithAddress:&iobtAddress];
    if (!device || !device.services) {
        if (singleDevice) {
            error = QBluetoothServiceDiscoveryAgent::UnknownError;
            errorString = QCoreApplication::translate(SERVICE_DISCOVERY, SD_MINIMAL_FAILED);
            emit q_ptr->errorOccurred(error);
        }
    } else {

        NSArray *const records = device.services;
        for (IOBluetoothSDPServiceRecord *record in records) {
            QBluetoothServiceInfo serviceInfo;
            Q_ASSERT_X(discoveredDevices.size() >= 1, Q_FUNC_INFO,
                       "invalid number of devices");

            serviceInfo.setDevice(discoveredDevices.at(0));
            DarwinBluetooth::extract_service_record(record, serviceInfo);

            if (!serviceInfo.isValid())
                continue;

            if (!uuidFilter.isEmpty() && !serviceHasMatchingUuid(serviceInfo))
                continue;

            if (!isDuplicatedService(serviceInfo)) {
                discoveredServices.append(serviceInfo);
                emit q_ptr->serviceDiscovered(serviceInfo);
            }
        }
    }

    _q_serviceDiscoveryFinished();
}

bool QBluetoothServiceDiscoveryAgentPrivate::serviceHasMatchingUuid(const QBluetoothServiceInfo &serviceInfo) const
{
    for (const auto &requestedUuid : uuidFilter) {
        if (serviceInfo.serviceUuid() == requestedUuid)
            return true;
        if (serviceInfo.serviceClassUuids().contains(requestedUuid))
            return true;
    }
    return false;
}

QT_END_NAMESPACE
