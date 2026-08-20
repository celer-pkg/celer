// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 BlackBerry Limited all rights reserved
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYCHARACTERISTIC_H
#define QLOWENERGYCHARACTERISTIC_H
#include <QtCore/QSharedPointer>
#include <QtCore/QObject>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/QBluetoothUuid>
#include <QtBluetooth/QLowEnergyDescriptor>

QT_BEGIN_NAMESPACE

class QBluetoothUuid;
class QLowEnergyServicePrivate;
struct QLowEnergyCharacteristicPrivate;
class Q_BLUETOOTH_EXPORT QLowEnergyCharacteristic
{
public:

    enum PropertyType {
        Unknown = 0x00,
        Broadcasting = 0x01,
        Read = 0x02,
        WriteNoResponse = 0x04,
        Write = 0x08,
        Notify = 0x10,
        Indicate = 0x20,
        WriteSigned = 0x40,
        ExtendedProperty = 0x80
    };
    Q_DECLARE_FLAGS(PropertyTypes, PropertyType)

    QLowEnergyCharacteristic();
    QLowEnergyCharacteristic(const QLowEnergyCharacteristic &other);
    ~QLowEnergyCharacteristic();

    QLowEnergyCharacteristic &operator=(const QLowEnergyCharacteristic &other);
    friend bool operator==(const QLowEnergyCharacteristic &a, const QLowEnergyCharacteristic &b)
    {
        return equals(a, b);
    }
    friend bool operator!=(const QLowEnergyCharacteristic &a, const QLowEnergyCharacteristic &b)
    {
        return !equals(a, b);
    }

    QString name() const;

    QBluetoothUuid uuid() const;

    QByteArray value() const;

    QLowEnergyCharacteristic::PropertyTypes properties() const;

    QLowEnergyDescriptor descriptor(const QBluetoothUuid &uuid) const;
    QList<QLowEnergyDescriptor> descriptors() const;

    QLowEnergyDescriptor clientCharacteristicConfiguration() const;

    bool isValid() const;

    static const QByteArray CCCDDisable;
    static const QByteArray CCCDEnableNotification;
    static const QByteArray CCCDEnableIndication;

private:
    QLowEnergyHandle handle() const;
    QLowEnergyHandle attributeHandle() const;

    QSharedPointer<QLowEnergyServicePrivate> d_ptr;

    friend class QLowEnergyService;
    friend class QLowEnergyControllerPrivate;
    friend class QLowEnergyControllerPrivateAndroid;
    friend class QLowEnergyControllerPrivateBluez;
    friend class QLowEnergyControllerPrivateBluezDBus;
    friend class QLowEnergyControllerPrivateCommon;
    friend class QLowEnergyControllerPrivateDarwin;
    friend class QLowEnergyControllerPrivateWinRT;
    QLowEnergyCharacteristicPrivate *data = nullptr;
    QLowEnergyCharacteristic(QSharedPointer<QLowEnergyServicePrivate> p,
                             QLowEnergyHandle handle);

    static bool equals(const QLowEnergyCharacteristic &a, const QLowEnergyCharacteristic &b);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QLowEnergyCharacteristic::PropertyTypes)

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN(QLowEnergyCharacteristic, Q_BLUETOOTH_EXPORT)

#endif // QLOWENERGYCHARACTERISTIC_H
