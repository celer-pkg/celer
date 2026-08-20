// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANBUSDEVICE_P_H
#define QCANBUSDEVICE_P_H

#include <QtCore/qmutex.h>
#include <QtSerialBus/qcanbusdevice.h>

#include <private/qobject_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

typedef QPair<QCanBusDevice::ConfigurationKey, QVariant > ConfigEntry;

class QCanBusDevicePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QCanBusDevice)
public:
    QCanBusDevicePrivate() {}

    QCanBusDevice::CanBusError lastError = QCanBusDevice::CanBusError::NoError;
    QCanBusDevice::CanBusDeviceState state = QCanBusDevice::UnconnectedState;
    QString errorText;

    QList<QCanBusFrame> incomingFrames;
    mutable QMutex incomingFramesGuard;
    QList<QCanBusFrame> outgoingFrames;
    QList<ConfigEntry> configOptions;

    bool waitForReceivedEntered = false;
    bool waitForWrittenEntered = false;

    std::function<void()> m_resetControllerFunction;
    std::function<QCanBusDevice::CanBusStatus()> m_busStatusGetter;
};

QT_END_NAMESPACE

#endif // QCANBUSDEVICE_P_H
