// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QLOWENERGYDESCRIPTORDATA_H
#define QLOWENERGYDESCRIPTORDATA_H

#include <QtBluetooth/qbluetooth.h>
#include <QtBluetooth/qbluetoothuuid.h>
#include <QtCore/qshareddata.h>

QT_BEGIN_NAMESPACE

class QByteArray;
struct QLowEnergyDescriptorDataPrivate;

class Q_BLUETOOTH_EXPORT QLowEnergyDescriptorData
{
public:
    QLowEnergyDescriptorData();
    QLowEnergyDescriptorData(const QBluetoothUuid &uuid,
                             const QByteArray &value);
    QLowEnergyDescriptorData(const QLowEnergyDescriptorData &other);
    ~QLowEnergyDescriptorData();

    QLowEnergyDescriptorData &operator=(const QLowEnergyDescriptorData &other);
    friend bool operator==(const QLowEnergyDescriptorData &a, const QLowEnergyDescriptorData &b)
    {
        return equals(a, b);
    }

    friend bool operator!=(const QLowEnergyDescriptorData &a, const QLowEnergyDescriptorData &b)
    {
        return !equals(a, b);
    }

    QByteArray value() const;
    void setValue(const QByteArray &value);

    QBluetoothUuid uuid() const;
    void setUuid(const QBluetoothUuid &uuid);

    bool isValid() const;

    void setReadPermissions(bool readable,
            QBluetooth::AttAccessConstraints constraints = QBluetooth::AttAccessConstraints());
    bool isReadable() const;
    QBluetooth::AttAccessConstraints readConstraints() const;

    void setWritePermissions(bool writable,
            QBluetooth::AttAccessConstraints constraints = QBluetooth::AttAccessConstraints());
    bool isWritable() const;
    QBluetooth::AttAccessConstraints writeConstraints() const;

    void swap(QLowEnergyDescriptorData &other) noexcept { d.swap(other.d); }

private:
    static bool equals(const QLowEnergyDescriptorData &a, const QLowEnergyDescriptorData &b);
    QSharedDataPointer<QLowEnergyDescriptorDataPrivate> d;
};

Q_DECLARE_SHARED(QLowEnergyDescriptorData)

QT_END_NAMESPACE

#endif // Include guard.
