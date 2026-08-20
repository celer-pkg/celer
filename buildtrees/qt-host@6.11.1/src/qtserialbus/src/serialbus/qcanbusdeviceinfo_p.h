// Copyright (C) 2021 Andre Hartmann <aha_1980@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCANBUSDEVICEINFO_P_H
#define QCANBUSDEVICEINFO_P_H

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

#include <QtCore/qshareddata.h>
#include <QtCore/qstring.h>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QCanBusDeviceInfoPrivate : public QSharedData {
public:
    QCanBusDeviceInfoPrivate() { }

    ~QCanBusDeviceInfoPrivate()
    {
    }

    QString plugin;
    QString name;
    QString description;
    QString serialNumber;
    QString alias;
    int  channel = 0;
    bool hasFlexibleDataRate = false;
    bool isVirtual = false;
};

QT_END_NAMESPACE

#endif // QCANBUSDEVICEINFO_P_H
