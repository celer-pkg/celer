// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QFINALSTATE_P_H
#define QFINALSTATE_P_H

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

#include "qfinalstate.h"
#include "private/qabstractstate_p.h"
#include <private/qstatemachineglobal_p.h>

QT_REQUIRE_CONFIG(statemachine);

QT_BEGIN_NAMESPACE

class Q_STATEMACHINE_EXPORT QFinalStatePrivate : public QAbstractStatePrivate
{
    Q_DECLARE_PUBLIC(QFinalState)

public:
    QFinalStatePrivate();
    ~QFinalStatePrivate();
};

QT_END_NAMESPACE

#endif // QFINALSTATE_P_H
