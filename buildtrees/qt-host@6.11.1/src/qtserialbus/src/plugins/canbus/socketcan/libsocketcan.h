// Copyright (C) 2019 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef LIBSOCKETCAN_H
#define LIBSOCKETCAN_H

#include <QtCore/qglobal.h>
#include <QtSerialBus/qcanbusdevice.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QString;

class LibSocketCan final
{
public:
    explicit LibSocketCan(QString *errorString = nullptr);

    bool start(const QString &interface);
    bool stop(const QString &interface);
    bool restart(const QString &interface);

    quint32 bitrate(const QString &interface) const;
    bool setBitrate(const QString &interface, quint32 bitrate);

    bool hasBusStatus() const;
    QCanBusDevice::CanBusStatus busStatus(const QString &interface) const;
};

QT_END_NAMESPACE

#endif // LIBSOCKETCAN_H
