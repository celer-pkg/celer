// Copyright (C) 2016 Research In Motion
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QSENSORBACKEND_P_H
#define QSENSORBACKEND_P_H

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

#include "qsensorbackend.h"

#include "private/qobject_p.h"

QT_BEGIN_NAMESPACE

class QSensorBackendPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSensorBackend)
public:
    explicit QSensorBackendPrivate(QSensor *sensor)
        : m_sensor(sensor)
    {
    }

    QSensor *m_sensor;
};

QT_END_NAMESPACE

#endif

