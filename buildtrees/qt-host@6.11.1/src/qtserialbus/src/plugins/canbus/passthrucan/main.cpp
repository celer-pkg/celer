// Copyright (C) 2017 Ford Motor Company.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "passthrucanbackend.h"

#include <QtSerialBus/qcanbus.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusfactory.h>

QT_BEGIN_NAMESPACE

class PassThruCanBusPlugin : public QObject, public QCanBusFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QCanBusFactory" FILE "plugin.json")
    Q_INTERFACES(QCanBusFactory)

public:
    PassThruCanBusPlugin()
    {
        qRegisterMetaType<QCanBusDevice::CanBusError>();
        qRegisterMetaType<QList<QCanBusFrame>>();
    }

    QList<QCanBusDeviceInfo> availableDevices(QString *) const override
    {
        return PassThruCanBackend::interfaces();
    }

    QCanBusDevice *createDevice(const QString &interfaceName, QString *) const override
    {
        return new PassThruCanBackend(interfaceName);
    }
};

QT_END_NAMESPACE

#include "main.moc"
