// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QTILTSENSOR_P_H
#define QTILTSENSOR_P_H

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

class QTiltReadingPrivate
{
public:
    QTiltReadingPrivate()
        : yRotation(0)
        , xRotation(0)
    {
    }

    qreal yRotation;
    qreal xRotation;
};

QT_END_NAMESPACE
#endif
