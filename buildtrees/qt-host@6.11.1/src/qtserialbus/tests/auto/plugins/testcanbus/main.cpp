// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "testcanbackend.h"

#include <QtSerialBus/qcanbus.h>
#include <QtSerialBus/qcanbusfactory.h>

QT_BEGIN_NAMESPACE

class TestCanBusPlugin : public QObject, public QCanBusFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QCanBusFactory" FILE "plugin.json")
    Q_INTERFACES(QCanBusFactory)

public:
    QList<QCanBusDeviceInfo> availableDevices(QString *errorMessage) const override
    {
        Q_UNUSED(errorMessage);

        return TestCanBackend::interfaces();
    }

    QCanBusDevice *createDevice(const QString &interfaceName, QString *errorMessage) const override
    {
        if (interfaceName == QStringLiteral("invalid")) {
            if (errorMessage)
                *errorMessage = tr("No such interface: '%1'").arg(interfaceName);

            return nullptr;
        }
        auto device = new TestCanBackend();
        return device;
    }
};

QT_END_NAMESPACE

#include "main.moc"
