// Copyright (C) 2016 The Qt Company Ltd.
// Copyright (C) 2016 BlackBerry Limited. All rights reserved.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYDESCRIPTOR_H
#define QLOWENERGYDESCRIPTOR_H

#include <QtCore/QSharedPointer>
#include <QtCore/QVariantMap>
#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/QBluetoothUuid>

QT_BEGIN_NAMESPACE

struct QLowEnergyDescriptorPrivate;
class QLowEnergyServicePrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyDescriptor
{
public:
    QLowEnergyDescriptor();
    QLowEnergyDescriptor(const QLowEnergyDescriptor &other);
    ~QLowEnergyDescriptor();

    QLowEnergyDescriptor &operator=(const QLowEnergyDescriptor &other);
    friend bool operator==(const QLowEnergyDescriptor &a, const QLowEnergyDescriptor &b)
    {
        return equals(a, b);
    }
    friend bool operator!=(const QLowEnergyDescriptor &a, const QLowEnergyDescriptor &b)
    {
        return !equals(a, b);
    }

    bool isValid() const;

    QByteArray value() const;

    QBluetoothUuid uuid() const;
    QString name() const;

    QBluetoothUuid::DescriptorType type() const;

private:
    QLowEnergyHandle handle() const;
    QLowEnergyHandle characteristicHandle() const;
    QSharedPointer<QLowEnergyServicePrivate> d_ptr;

    friend class QLowEnergyCharacteristic;
    friend class QLowEnergyService;
    friend class QLowEnergyControllerPrivate;
    friend class QLowEnergyControllerPrivateAndroid;
    friend class QLowEnergyControllerPrivateBluez;
    friend class QLowEnergyControllerPrivateBluezDBus;
    friend class QLowEnergyControllerPrivateCommon;
    friend class QLowEnergyControllerPrivateDarwin;
    friend class QLowEnergyControllerPrivateWinRT;
    QLowEnergyDescriptorPrivate *data = nullptr;

    QLowEnergyDescriptor(QSharedPointer<QLowEnergyServicePrivate> p,
                             QLowEnergyHandle charHandle,
                             QLowEnergyHandle descHandle);

    static bool equals(const QLowEnergyDescriptor &a, const QLowEnergyDescriptor &b);
};

QT_END_NAMESPACE

QT_DECL_METATYPE_EXTERN(QLowEnergyDescriptor, Q_BLUETOOTH_EXPORT)

#endif // QLOWENERGYDESCRIPTOR_H
