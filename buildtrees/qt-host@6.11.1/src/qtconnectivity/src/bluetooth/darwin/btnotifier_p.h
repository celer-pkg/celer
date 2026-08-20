// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef BTNOTIFIER_P_H
#define BTNOTIFIER_P_H

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

#include "qbluetoothdevicediscoveryagent.h"
#include "qlowenergycontroller.h"
#include "qbluetoothdeviceinfo.h"
#include "qbluetoothuuid.h"
#include "qbluetooth.h"

#include <QtCore/qsharedpointer.h>
#include <QtCore/qbytearray.h>
#include <QtCore/private/qglobal_p.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QLowEnergyServicePrivate;

namespace DarwinBluetooth
{

class LECBManagerNotifier : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void deviceDiscovered(QBluetoothDeviceInfo deviceInfo);
    void discoveryFinished();

    void connected();
    void disconnected();

    void mtuChanged(int newValue);

    void serviceDiscoveryFinished();
    void serviceDetailsDiscoveryFinished(QSharedPointer<QLowEnergyServicePrivate> service);
    void characteristicRead(QLowEnergyHandle charHandle, const QByteArray &value);
    void characteristicWritten(QLowEnergyHandle charHandle, const QByteArray &value);
    void characteristicUpdated(QLowEnergyHandle charHandle, const QByteArray &value);
    void descriptorRead(QLowEnergyHandle descHandle, const QByteArray &value);
    void descriptorWritten(QLowEnergyHandle descHandle, const QByteArray &value);
    void notificationEnabled(QLowEnergyHandle charHandle, bool enabled);
    void servicesWereModified();
    void rssiUpdated(qint16 newValue);

    void LEnotSupported();
    void CBManagerError(QBluetoothDeviceDiscoveryAgent::Error error);
    void CBManagerError(QLowEnergyController::Error error);
    void CBManagerError(const QBluetoothUuid &serviceUuid, QLowEnergyController::Error error);
    void CBManagerError(const QBluetoothUuid &serviceUuid, QLowEnergyService::ServiceError error);
};

} // namespace DarwinBluetooth

QT_END_NAMESPACE

#endif
