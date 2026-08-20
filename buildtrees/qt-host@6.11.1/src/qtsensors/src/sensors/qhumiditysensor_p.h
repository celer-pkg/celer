// Copyright (C) 2016 Canonical Ltd
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QHUMIDITYSENSOR_P_H
#define QHUMIDITYSENSOR_P_H

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

#include "qsensor_p.h"

QT_BEGIN_NAMESPACE

class QHumidityReadingPrivate
{
public:
    QHumidityReadingPrivate()
        : relativeHumidity(0),
          absoluteHumidity(0)
    {
    }

    qreal relativeHumidity;
    qreal absoluteHumidity;
};

class QHumiditySensorPrivate : public QSensorPrivate
{
public:
    QHumiditySensorPrivate()
    {
    }

};

QT_END_NAMESPACE

#endif
