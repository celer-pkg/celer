// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANBUSFACTORY_H
#define QCANBUSFACTORY_H

#include <QtCore/qstringlist.h>
#include <QtSerialBus/qtserialbusglobal.h>
#include <QtSerialBus/qcanbusdevice.h>
#include <QtSerialBus/qcanbusdeviceinfo.h>

QT_BEGIN_NAMESPACE

class Q_SERIALBUS_EXPORT QCanBusFactory
{
public:
    virtual QCanBusDevice *createDevice(const QString &interfaceName,
                                QString *errorMessage) const = 0;
    virtual QList<QCanBusDeviceInfo> availableDevices(QString *errorMessage) const = 0;

protected:
    virtual ~QCanBusFactory();
};

Q_DECLARE_INTERFACE(QCanBusFactory, "org.qt-project.Qt.QCanBusFactory")

QT_END_NAMESPACE

#endif // QCANBUSFACTORY_H

