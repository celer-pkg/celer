// Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef TINYCANBACKEND_H
#define TINYCANBACKEND_H

#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>
#include <QtSerialBus/qcanbusframe.h>

#include <QtCore/qlist.h>
#include <QtCore/qvariant.h>

QT_BEGIN_NAMESPACE

class TinyCanBackendPrivate;

class TinyCanBackend : public QCanBusDevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(TinyCanBackend)
    Q_DISABLE_COPY(TinyCanBackend)
public:
    explicit TinyCanBackend(const QString &name, QObject *parent = nullptr);
    ~TinyCanBackend();

    bool open() override;
    void close() override;

    void setConfigurationParameter(ConfigurationKey key, const QVariant &value) override;

    bool writeFrame(const QCanBusFrame &newData) override;

    QString interpretErrorFrame(const QCanBusFrame &errorFrame) override;

    static bool canCreate(QString *errorReason);
    static QList<QCanBusDeviceInfo> interfaces();

    void resetController() override;
    QCanBusDeviceInfo deviceInfo() const override;

private:
    TinyCanBackendPrivate * const d_ptr;
};

QT_END_NAMESPACE

#endif // TINYCANBACKEND_H
