// Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef SYSTECCANBACKEND_H
#define SYSTECCANBACKEND_H

#include <QtSerialBus/qcanbusframe.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

#include <QtCore/qvariant.h>
#include <QtCore/qlist.h>

QT_BEGIN_NAMESPACE

class SystecCanBackendPrivate;

class SystecCanBackend : public QCanBusDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SystecCanBackend)
    Q_DISABLE_COPY(SystecCanBackend)
public:
    explicit SystecCanBackend(const QString &name, QObject *parent = nullptr);
    ~SystecCanBackend();

    bool open() override;
    void close() override;

    void setConfigurationParameter(ConfigurationKey key, const QVariant &value) override;

    bool writeFrame(const QCanBusFrame &newData) override;

    QString interpretErrorFrame(const QCanBusFrame &errorFrame) override;

    static QList<QCanBusDeviceInfo> interfaces();
    static bool canCreate(QString *errorReason);

    // This function needs to be public as it is accessed by a callback
    static QCanBusDeviceInfo createDeviceInfo(const QString &serialNumber,
                                              const QString &description,
                                              uint deviceNumber,
                                              int channelNumber);

    void resetController() override;
    bool hasBusStatus() const override;
    CanBusStatus busStatus() override;
    QCanBusDeviceInfo deviceInfo() const override;

private:
    SystecCanBackendPrivate * const d_ptr;
};

QT_END_NAMESPACE

#endif // SYSTECCANBACKEND_H
