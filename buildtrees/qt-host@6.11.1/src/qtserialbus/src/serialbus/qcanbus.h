// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANBUS_H
#define QCANBUS_H

#include <QtCore/qobject.h>
#include <QtSerialBus/qtserialbusglobal.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QCanBus : public QObject
{
    Q_OBJECT

public:
    static QCanBus *instance();
    QStringList plugins() const;

    QList<QCanBusDeviceInfo> availableDevices(const QString &plugin, QString *errorMessage = nullptr) const;
    QList<QCanBusDeviceInfo> availableDevices(QString *errorMessage = nullptr) const;

    QCanBusDevice *createDevice(const QString &plugin,
                                const QString &interfaceName,
                                QString *errorMessage = nullptr) const;

private:
    QCanBus(QObject *parent = nullptr);

    Q_DISABLE_COPY(QCanBus)
};

QT_END_NAMESPACE

#endif // QSERIALBUS_H
