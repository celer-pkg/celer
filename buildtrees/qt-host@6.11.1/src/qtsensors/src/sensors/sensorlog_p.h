// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QSENSORLOG_P_H
#define QSENSORLOG_P_H

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

#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

#ifdef ENABLE_RUNTIME_SENSORLOG

static bool logEnabled()
{
    static int state = -1;

    if (state == -1) {
        QByteArray sensorlog = qgetenv("SENSORLOG");
        if (sensorlog == "1")
            state = 1;
        else
            state = 0;
    }

    return state;
}

#define SENSORLOG() if (!logEnabled()); else qDebug()

#else

// Do nothing (compiles to almost nothing)
#define SENSORLOG() if (1); else qDebug()

#endif

QT_END_NAMESPACE

#endif

