// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYSERVICEDATA_H
#define QLOWENERGYSERVICEDATA_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QBluetoothUuid;
class QLowEnergyCharacteristicData;
class QLowEnergyService;
struct QLowEnergyServiceDataPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyServiceData
{
public:
    QLowEnergyServiceData();
    QLowEnergyServiceData(const QLowEnergyServiceData &other);
    ~QLowEnergyServiceData();

    QLowEnergyServiceData &operator=(const QLowEnergyServiceData &other);
    friend bool operator==(const QLowEnergyServiceData &a, const QLowEnergyServiceData &b)
    {
        return equals(a, b);
    }
    friend bool operator!=(const QLowEnergyServiceData &a, const QLowEnergyServiceData &b)
    {
        return !equals(a, b);
    }

    enum ServiceType { ServiceTypePrimary = 0x2800, ServiceTypeSecondary = 0x2801 };
    ServiceType type() const;
    void setType(ServiceType type);

    QBluetoothUuid uuid() const;
    void setUuid(const QBluetoothUuid &uuid);

    QList<QLowEnergyService *> includedServices() const;
    void setIncludedServices(const QList<QLowEnergyService *> &services);
    void addIncludedService(QLowEnergyService *service);

    QList<QLowEnergyCharacteristicData> characteristics() const;
    void setCharacteristics(const QList<QLowEnergyCharacteristicData> &characteristics);
    void addCharacteristic(const QLowEnergyCharacteristicData &characteristic);

    bool isValid() const;

    void swap(QLowEnergyServiceData &other) noexcept { d.swap(other.d); }

private:
    static bool equals(const QLowEnergyServiceData &a, const QLowEnergyServiceData &b);
    QSharedDataPointer<QLowEnergyServiceDataPrivate> d;
};

Q_DECLARE_SHARED(QLowEnergyServiceData)

QT_END_NAMESPACE

#endif // Include guard.
