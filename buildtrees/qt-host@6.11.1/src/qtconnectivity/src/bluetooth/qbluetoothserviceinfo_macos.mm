// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qbluetoothserviceinfo_p.h"
#include "darwin/btservicerecord_p.h"
#include "qbluetoothserviceinfo.h"
#include "qbluetoothdeviceinfo.h"
#include "qbluetoothserver_p.h"
#include "darwin/btutility_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qvariant.h>
#include <QtCore/qglobal.h>
#include <QtCore/qmutex.h>
#include <QtCore/qmap.h>
#include <QtCore/qurl.h>

#include <Foundation/Foundation.h>

#include <IOBluetooth/IOBluetooth.h>

QT_BEGIN_NAMESPACE

namespace {

using DarwinBluetooth::RetainPolicy;
using ServiceInfo = QBluetoothServiceInfo;

// Alas, since there is no d_ptr<->q_ptr link (which is not that bad in itself),
// I need these getters duplicated here:
ServiceInfo::Protocol socket_protocol(const QBluetoothServiceInfoPrivate &privateInfo)
{
    ServiceInfo::Sequence parameters = privateInfo.protocolDescriptor(QBluetoothUuid::ProtocolUuid::Rfcomm);
    if (!parameters.isEmpty())
        return ServiceInfo::RfcommProtocol;

    parameters = privateInfo.protocolDescriptor(QBluetoothUuid::ProtocolUuid::L2cap);
    if (!parameters.isEmpty())
        return ServiceInfo::L2capProtocol;

    return ServiceInfo::UnknownProtocol;
}

int channel_or_psm(const QBluetoothServiceInfoPrivate &privateInfo, QBluetoothUuid::ProtocolUuid uuid)
{
    const auto parameters = privateInfo.protocolDescriptor(uuid);
    if (parameters.isEmpty())
        return -1;
    else if (parameters.size() == 1)
        return 0;

    return parameters.at(1).toInt();
}

} // unnamed namespace

QBluetoothServiceInfoPrivate::QBluetoothServiceInfoPrivate()
{
}

QBluetoothServiceInfoPrivate::~QBluetoothServiceInfoPrivate()
{
}

bool QBluetoothServiceInfoPrivate::registerService(const QBluetoothAddress &localAddress)
{
    Q_UNUSED(localAddress);
    return false;
}

bool QBluetoothServiceInfoPrivate::registerService(const QBluetoothServiceInfo &info)
{
    using namespace DarwinBluetooth;

    if (isRegistered())
        return false;

    using namespace DarwinBluetooth;

    ObjCStrongReference<NSMutableDictionary> serviceDict(iobluetooth_service_dictionary(info));
    if (!serviceDict) {
        qCWarning(QT_BT_DARWIN) << "failed to create a service dictionary";
        return false;
    }

    Q_ASSERT(!registered);
    Q_ASSERT_X(!serviceRecord, Q_FUNC_INFO, "not registered, but serviceRecord is not nil");

    SDPRecord newRecord;
    newRecord.reset([IOBluetoothSDPServiceRecord
                     publishedServiceRecordWithDictionary:serviceDict], RetainPolicy::doInitialRetain);
    if (!newRecord) {
        qCWarning(QT_BT_DARWIN) << "failed to register a service record";
        return false;
    }

    BluetoothSDPServiceRecordHandle newRecordHandle = 0;
    auto *ioSDPRecord = newRecord.getAs<IOBluetoothSDPServiceRecord>();
    if ([ioSDPRecord getServiceRecordHandle:&newRecordHandle] != kIOReturnSuccess) {
        qCWarning(QT_BT_DARWIN) << "failed to register a service record";
        [ioSDPRecord removeServiceRecord];
        return false;
    }

    const ServiceInfo::Protocol type = info.socketProtocol();
    quint16 realPort = 0;
    QBluetoothServerPrivate *server = nullptr;
    bool configured = false;

    if (type == QBluetoothServiceInfo::L2capProtocol) {
        BluetoothL2CAPPSM psm = 0;
        server = QBluetoothServerPrivate::registeredServer(info.protocolServiceMultiplexer(), type);
        if ([ioSDPRecord getL2CAPPSM:&psm] == kIOReturnSuccess) {
            configured = true;
            realPort = psm;
        }
    } else if (type == QBluetoothServiceInfo::RfcommProtocol) {
        BluetoothRFCOMMChannelID channelID = 0;
        server = QBluetoothServerPrivate::registeredServer(info.serverChannel(), type);
        if ([ioSDPRecord getRFCOMMChannelID:&channelID] == kIOReturnSuccess) {
            configured = true;
            realPort = channelID;
        }
    }

    if (!configured) {
        [ioSDPRecord removeServiceRecord];
        qCWarning(QT_BT_DARWIN) << "failed to register a service record";
        return false;
    }

    registered = true;
    serviceRecord.swap(newRecord);
    serviceRecordHandle = newRecordHandle;

    if (server)
        server->startListener(realPort);

    return true;
}

bool QBluetoothServiceInfoPrivate::isRegistered() const
{
    return registered;
}

bool QBluetoothServiceInfoPrivate::unregisterService()
{
    if (!registered)
        return false;

    Q_ASSERT_X(serviceRecord, Q_FUNC_INFO, "service registered, but serviceRecord is nil");

    auto *nativeRecord = serviceRecord.getAs<IOBluetoothSDPServiceRecord>();
    [nativeRecord removeServiceRecord];
    serviceRecord.reset();

    const ServiceInfo::Protocol type = socket_protocol(*this);
    QBluetoothServerPrivate *server = nullptr;

    const QMutexLocker lock(&QBluetoothServerPrivate::channelMapMutex());
    if (type == ServiceInfo::RfcommProtocol)
        server = QBluetoothServerPrivate::registeredServer(channel_or_psm(*this, QBluetoothUuid::ProtocolUuid::Rfcomm), type);
    else if (type == ServiceInfo::L2capProtocol)
        server = QBluetoothServerPrivate::registeredServer(channel_or_psm(*this, QBluetoothUuid::ProtocolUuid::L2cap), type);

    if (server)
        server->stopListener();

    registered = false;
    serviceRecordHandle = 0;

    return true;
}

QT_END_NAMESPACE
