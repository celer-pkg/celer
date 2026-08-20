// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QLOWENERGYCHARACTERISTICDATA_H
#define QLOWENERGYCHARACTERISTICDATA_H

#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QLowEnergyDescriptorData;
struct QLowEnergyCharacteristicDataPrivate;
class Q_BLUETOOTH_EXPORT QLowEnergyCharacteristicData
{
public:
    QLowEnergyCharacteristicData();
    QLowEnergyCharacteristicData(const QLowEnergyCharacteristicData &other);
    ~QLowEnergyCharacteristicData();

    QLowEnergyCharacteristicData &operator=(const QLowEnergyCharacteristicData &other);
    friend bool operator==(const QLowEnergyCharacteristicData &a,
                           const QLowEnergyCharacteristicData &b)
    {
        return equals(a, b);
    }
    friend bool operator!=(const QLowEnergyCharacteristicData &a,
                           const QLowEnergyCharacteristicData &b)
    {
        return !equals(a, b);
    }

    QBluetoothUuid uuid() const;
    void setUuid(const QBluetoothUuid &uuid);

    QByteArray value() const;
    void setValue(const QByteArray &value);

    QLowEnergyCharacteristic::PropertyTypes properties() const;
    void setProperties(QLowEnergyCharacteristic::PropertyTypes properties);

    QList<QLowEnergyDescriptorData> descriptors() const;
    void setDescriptors(const QList<QLowEnergyDescriptorData> &descriptors);
    void addDescriptor(const QLowEnergyDescriptorData &descriptor);

    void setReadConstraints(QBluetooth::AttAccessConstraints constraints);
    QBluetooth::AttAccessConstraints readConstraints() const;

    void setWriteConstraints(QBluetooth::AttAccessConstraints constraints);
    QBluetooth::AttAccessConstraints writeConstraints() const;

    void setValueLength(int minimum, int maximum);
    int minimumValueLength() const;
    int maximumValueLength() const;

    bool isValid() const;

    void swap(QLowEnergyCharacteristicData &other) noexcept { d.swap(other.d); }

private:
    static bool equals(const QLowEnergyCharacteristicData &a,
                       const QLowEnergyCharacteristicData &b);
    QSharedDataPointer<QLowEnergyCharacteristicDataPrivate> d;
};

Q_DECLARE_SHARED(QLowEnergyCharacteristicData)

QT_END_NAMESPACE

#endif // Include guard.
