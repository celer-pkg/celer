// Copyright (C) 2017 Ford Motor Company.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef PASSTHRUCAN_PASSTHRUCANBACKEND_H
#define PASSTHRUCAN_PASSTHRUCANBACKEND_H

#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusframe.h>

#include <QList>
#include <QString>
#include <QThread>

QT_BEGIN_NAMESPACE

class PassThruCanIO;

class PassThruCanBackend : public QCanBusDevice
{
    Q_OBJECT
    Q_DISABLE_COPY(PassThruCanBackend)
public:
    explicit PassThruCanBackend(const QString &name, QObject *parent = nullptr);
    virtual ~PassThruCanBackend();

    void setConfigurationParameter(ConfigurationKey key, const QVariant &value) override;
    bool writeFrame(const QCanBusFrame &frame) override;
    QString interpretErrorFrame(const QCanBusFrame &errorFrame) override;

    static QList<QCanBusDeviceInfo> interfaces();
    QCanBusDeviceInfo deviceInfo() const override;

protected:
    bool open() override;
    void close() override;

private:
    void ackOpenFinished(bool success);
    void ackCloseFinished();
    void applyConfig(QCanBusDevice::ConfigurationKey key, const QVariant &value);

    QString         m_deviceName;
    QThread         m_ioThread;
    PassThruCanIO * m_canIO;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QCanBusDevice::CanBusError)
Q_DECLARE_METATYPE(QList<QCanBusFrame>)

#endif // PASSTHRUCAN_PASSTHRUCANBACKEND_H
