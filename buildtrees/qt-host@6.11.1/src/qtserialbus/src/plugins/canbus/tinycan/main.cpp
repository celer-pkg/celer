// Copyright (C) 2017 Denis Shienkov <denis.shienkov@gmail.com>
// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "tinycanbackend.h"

#include <QtSerialBus/qcanbus.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusfactory.h>

#include <QtCore/qloggingcategory.h>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(QT_CANBUS_PLUGINS_TINYCAN, "qt.canbus.plugins.tinycan")

class TinyCanBusPlugin : public QObject, public QCanBusFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QCanBusFactory" FILE "plugin.json")
    Q_INTERFACES(QCanBusFactory)

public:
    QList<QCanBusDeviceInfo> availableDevices(QString *errorMessage) const override
    {
        Q_UNUSED(errorMessage);
        return TinyCanBackend::interfaces();
    }

    QCanBusDevice *createDevice(const QString &interfaceName, QString *errorMessage) const override
    {
        QString errorReason;
        if (!TinyCanBackend::canCreate(&errorReason)) {
            qCWarning(QT_CANBUS_PLUGINS_TINYCAN, "%ls", qUtf16Printable(errorReason));
            if (errorMessage)
                *errorMessage = errorReason;
            return nullptr;
        }

        auto device = new TinyCanBackend(interfaceName);
        return device;
    }
};

QT_END_NAMESPACE

#include "main.moc"
