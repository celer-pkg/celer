// Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANBUSDEVICEINFO_H
#define QCANBUSDEVICEINFO_H

#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtSerialBus/qtserialbusglobal.h>

QT_BEGIN_NAMESPACE

class QCanBusDeviceInfoPrivate;

QT_DECLARE_QSDP_SPECIALIZATION_DTOR(QCanBusDeviceInfoPrivate)

class Q_SERIALBUS_EXPORT QCanBusDeviceInfo
{
public:
    QCanBusDeviceInfo() = delete;
    QCanBusDeviceInfo(const QCanBusDeviceInfo &other);
    QCanBusDeviceInfo(QCanBusDeviceInfo &&other) noexcept = default;
    ~QCanBusDeviceInfo();

    void swap(QCanBusDeviceInfo &other) noexcept
    {
         d_ptr.swap(other.d_ptr);
    }

    QCanBusDeviceInfo &operator=(const QCanBusDeviceInfo &other);
    QCanBusDeviceInfo &operator=(QCanBusDeviceInfo &&other) noexcept
    {
        swap(other);
        return *this;
    }

    QString plugin() const;
    QString name() const;
    QString description() const;
    QString serialNumber() const;
    QString alias() const;
    int channel() const;

    bool hasFlexibleDataRate() const;
    bool isVirtual() const;

private:
    friend class QCanBusDevice;

    explicit QCanBusDeviceInfo(QCanBusDeviceInfoPrivate &dd);

    QSharedDataPointer<QCanBusDeviceInfoPrivate> d_ptr;
};

Q_DECLARE_SHARED(QCanBusDeviceInfo)

QT_END_NAMESPACE

#endif // QCANBUSDEVICEINFO_H
