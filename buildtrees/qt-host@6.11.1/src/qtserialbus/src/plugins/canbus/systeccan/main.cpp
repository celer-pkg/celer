// Copyright (C) 2017 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "systeccanbackend.h"

#include <QtSerialBus/qcanbus.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusfactory.h>

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_SYSTECCAN, "qt.canbus.plugins.systeccan")

class SystecCanBusPlugin : public QObject, public QCanBusFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QCanBusFactory" FILE "plugin.json")
    Q_INTERFACES(QCanBusFactory)

public:
    QList<QCanBusDeviceInfo> availableDevices(QString *errorMessage) const override
    {
        if (Q_UNLIKELY(!SystecCanBackend::canCreate(errorMessage)))
            return QList<QCanBusDeviceInfo>();

        return SystecCanBackend::interfaces();
    }

    QCanBusDevice *createDevice(const QString &interfaceName, QString *errorMessage) const override
    {
        QString errorReason;
        if (Q_UNLIKELY(!SystecCanBackend::canCreate(&errorReason))) {
            qCWarning(QT_CANBUS_PLUGINS_SYSTECCAN, "%ls", qUtf16Printable(errorReason));
            if (errorMessage)
                *errorMessage = errorReason;
            return nullptr;
        }

        auto *device = new SystecCanBackend(interfaceName);
        return device;
    }
};

QT_END_NAMESPACE

#include "main.moc"
