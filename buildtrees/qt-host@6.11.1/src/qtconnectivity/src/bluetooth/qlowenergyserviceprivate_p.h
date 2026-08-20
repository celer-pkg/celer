// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYSERVICEPRIVATE_P_H
#define QLOWENERGYSERVICEPRIVATE_P_H

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

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/QLowEnergyService>
#include <QtBluetooth/QLowEnergyCharacteristic>
#include <QtCore/private/qglobal_p.h>

#if defined(QT_ANDROID_BLUETOOTH)
#include <QtCore/QJniObject>
#endif

QT_BEGIN_NAMESPACE

class QLowEnergyControllerPrivate;

class QLowEnergyServicePrivate : public QObject
{
    Q_OBJECT
public:
    explicit QLowEnergyServicePrivate(QObject *parent = nullptr);
    ~QLowEnergyServicePrivate();

    struct DescData {
        QByteArray value;
        QBluetoothUuid uuid;
    };

    struct CharData {
        QLowEnergyHandle valueHandle;
        QBluetoothUuid uuid;
        QLowEnergyCharacteristic::PropertyTypes properties;
        QByteArray value;
        QHash<QLowEnergyHandle, DescData> descriptorList;
    };

    enum GattAttributeTypes {
        PrimaryService = 0x2800,
        SecondaryService = 0x2801,
        IncludeAttribute = 0x2802,
        Characteristic = 0x2803
    };

    void setController(QLowEnergyControllerPrivate* control);
    void setError(QLowEnergyService::ServiceError newError);
    void setState(QLowEnergyService::ServiceState newState);

signals:
    void stateChanged(QLowEnergyService::ServiceState newState);
    void errorOccurred(QLowEnergyService::ServiceError error);
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                               const QByteArray &newValue);
    void characteristicRead(const QLowEnergyCharacteristic &info,
                            const QByteArray &value);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                               const QByteArray &newValue);
    void descriptorRead(const QLowEnergyDescriptor &info,
                        const QByteArray &value);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor,
                           const QByteArray &newValue);

public:
    QLowEnergyHandle startHandle = 0;
    QLowEnergyHandle endHandle = 0;

    QBluetoothUuid uuid;
    QList<QBluetoothUuid> includedServices;
    QLowEnergyService::ServiceTypes type = QLowEnergyService::PrimaryService;
    QLowEnergyService::ServiceState state = QLowEnergyService::InvalidService;
    QLowEnergyService::ServiceError lastError = QLowEnergyService::NoError;
    QLowEnergyService::DiscoveryMode mode = QLowEnergyService::FullDiscovery;

    QHash<QLowEnergyHandle, CharData> characteristicList;

    QPointer<QLowEnergyControllerPrivate> controller;

#if defined(QT_ANDROID_BLUETOOTH)
    // reference to the BluetoothGattService object
    QJniObject androidService;
#endif

};

typedef QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> CharacteristicDataMap;
typedef QHash<QLowEnergyHandle, QLowEnergyServicePrivate::DescData> DescriptorDataMap;

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN_TAGGED(QSharedPointer<QLowEnergyServicePrivate>,
                               QSharedPointer_QLowEnergyServicePrivate,
                               /* not exported */)

#endif // QLOWENERGYSERVICEPRIVATE_P_H
