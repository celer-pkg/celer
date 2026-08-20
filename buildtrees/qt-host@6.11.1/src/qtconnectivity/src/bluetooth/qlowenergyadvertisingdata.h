// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYADVERTISINGDATA_H
#define QLOWENERGYADVERTISINGDATA_H

#include <QtBluetooth/qtbluetoothglobal.h>
#include <QtBluetooth/qbluetoothuuid.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QLowEnergyAdvertisingDataPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyAdvertisingData
{
public:
    QLowEnergyAdvertisingData();
    QLowEnergyAdvertisingData(const QLowEnergyAdvertisingData &other);
    ~QLowEnergyAdvertisingData();

    QLowEnergyAdvertisingData &operator=(const QLowEnergyAdvertisingData &other);
    friend bool operator==(const QLowEnergyAdvertisingData &a, const QLowEnergyAdvertisingData &b)
    {
        return equals(a, b);
    }
    friend bool operator!=(const QLowEnergyAdvertisingData &a, const QLowEnergyAdvertisingData &b)
    {
        return !equals(a, b);
    }

    void setLocalName(const QString &name);
    QString localName() const;

    static quint16 invalidManufacturerId() { return 0xffff; }
    void setManufacturerData(quint16 id, const QByteArray &data);
    quint16 manufacturerId() const;
    QByteArray manufacturerData() const;

    void setIncludePowerLevel(bool doInclude);
    bool includePowerLevel() const;

    enum Discoverability {
        DiscoverabilityNone, DiscoverabilityLimited, DiscoverabilityGeneral
    };
    void setDiscoverability(Discoverability mode);
    Discoverability discoverability() const;

    void setServices(const QList<QBluetoothUuid> &services);
    QList<QBluetoothUuid> services() const;

    // TODO: BR/EDR capability flag?

    void setRawData(const QByteArray &data);
    QByteArray rawData() const;

    void swap(QLowEnergyAdvertisingData &other) noexcept { d.swap(other.d); }

private:
    static bool equals(const QLowEnergyAdvertisingData &a, const QLowEnergyAdvertisingData &b);
    QSharedDataPointer<QLowEnergyAdvertisingDataPrivate> d;
};

Q_DECLARE_SHARED(QLowEnergyAdvertisingData)

QT_END_NAMESPACE

#endif // Include guard
