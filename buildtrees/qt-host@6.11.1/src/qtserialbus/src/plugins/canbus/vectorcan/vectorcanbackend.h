// Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef VECTORCANBACKEND_H
#define VECTORCANBACKEND_H

#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>
#include <QtSerialBus/qcanbusframe.h>

#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class VectorCanBackendPrivate;

class VectorCanBackend : public QCanBusDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(VectorCanBackend)
    Q_DISABLE_COPY(VectorCanBackend)
public:
    explicit VectorCanBackend(const QString &name, QObject *parent = nullptr);
    ~VectorCanBackend();

    bool open() override;
    void close() override;

    void setConfigurationParameter(ConfigurationKey key, const QVariant &value) override;

    bool writeFrame(const QCanBusFrame &newData) override;

    QString interpretErrorFrame(const QCanBusFrame &errorFrame) override;

    static bool canCreate(QString *errorReason);
    static QList<QCanBusDeviceInfo> interfaces();

    bool hasBusStatus() const override;
    CanBusStatus busStatus() override;
    QCanBusDeviceInfo deviceInfo() const override;

private:
    VectorCanBackendPrivate * const d_ptr;
};

QT_END_NAMESPACE

#endif // VECTORCANBACKEND_H
