// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qlowenergycontroller_dummy_p.h"
#include "dummy/dummy_helper_p.h"

QT_BEGIN_NAMESPACE

QLowEnergyControllerPrivateCommon::QLowEnergyControllerPrivateCommon()
    : QLowEnergyControllerPrivate()
{
    printDummyWarning();
    registerQLowEnergyControllerMetaType();
}

QLowEnergyControllerPrivateCommon::~QLowEnergyControllerPrivateCommon()
{
}

void QLowEnergyControllerPrivateCommon::init()
{
}

void QLowEnergyControllerPrivateCommon::connectToDevice()
{
    // required to pass unit test on default backend
    if (remoteDevice.isNull()) {
        qWarning() << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    qWarning() << "QLowEnergyControllerPrivateCommon::connectToDevice(): Not implemented";
    setError(QLowEnergyController::UnknownError);
}

void QLowEnergyControllerPrivateCommon::disconnectFromDevice()
{

}

void QLowEnergyControllerPrivateCommon::discoverServices()
{

}

void QLowEnergyControllerPrivateCommon::discoverServiceDetails(
        const QBluetoothUuid & /*service*/, QLowEnergyService::DiscoveryMode /*mode*/)
{

}

void QLowEnergyControllerPrivateCommon::readCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
                        const QLowEnergyHandle /*charHandle*/)
{

}

void QLowEnergyControllerPrivateCommon::readDescriptor(const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
                    const QLowEnergyHandle /*charHandle*/,
                    const QLowEnergyHandle /*descriptorHandle*/)
{

}

void QLowEnergyControllerPrivateCommon::writeCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
        const QLowEnergyHandle /*charHandle*/,
        const QByteArray &/*newValue*/,
        QLowEnergyService::WriteMode /*writeMode*/)
{

}

void QLowEnergyControllerPrivateCommon::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> /*service*/,
        const QLowEnergyHandle /*charHandle*/,
        const QLowEnergyHandle /*descriptorHandle*/,
        const QByteArray &/*newValue*/)
{

}

void QLowEnergyControllerPrivateCommon::startAdvertising(const QLowEnergyAdvertisingParameters &/* params */,
        const QLowEnergyAdvertisingData &/* advertisingData */,
        const QLowEnergyAdvertisingData &/* scanResponseData */)
{
}

void QLowEnergyControllerPrivateCommon::stopAdvertising()
{
}

void QLowEnergyControllerPrivateCommon::requestConnectionUpdate(const QLowEnergyConnectionParameters & /* params */)
{
}

void QLowEnergyControllerPrivateCommon::addToGenericAttributeList(const QLowEnergyServiceData &/* service */,
                                                            QLowEnergyHandle /* startHandle */)
{
}

int QLowEnergyControllerPrivateCommon::mtu() const
{
    // not supported
    return -1;
}

QT_END_NAMESPACE
