// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGYROSCOPE_P_H
#define QGYROSCOPE_P_H

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

class QGyroscopeReadingPrivate
{
public:
    QGyroscopeReadingPrivate()
        : x(0)
        , y(0)
        , z(0)
    {
    }

    qreal x;
    qreal y;
    qreal z;
};

QT_END_NAMESPACE

#endif

