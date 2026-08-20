// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:insignificant

#ifndef STATEMACHINEFOREIGN_H
#define STATEMACHINEFOREIGN_H

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

#include "qstatemachineqmlglobals_p.h"

#include <QtQml/qqml.h>
#include <QtStateMachine/qhistorystate.h>
#include <QtStateMachine/qstate.h>
#include <QtStateMachine/qabstractstate.h>
#include <QtStateMachine/qsignaltransition.h>

struct Q_STATEMACHINEQML_EXPORT QHistoryStateForeign
{
    Q_GADGET
    QML_FOREIGN(QHistoryState)
    QML_NAMED_ELEMENT(HistoryState)
    QML_ADDED_IN_VERSION(1, 0)
};

struct Q_STATEMACHINEQML_EXPORT QStateForeign
{
    Q_GADGET
    QML_FOREIGN(QState)
    QML_NAMED_ELEMENT(QState)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("Don't use this, use State instead.")
};

struct Q_STATEMACHINEQML_EXPORT QAbstractStateForeign
{
    Q_GADGET
    QML_FOREIGN(QAbstractState)
    QML_NAMED_ELEMENT(QAbstractState)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("Don't use this, use State instead.")
};

struct Q_STATEMACHINEQML_EXPORT QSignalTransitionForeign
{
    Q_GADGET
    QML_FOREIGN(QSignalTransition)
    QML_NAMED_ELEMENT(QSignalTransition)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("Don't use this, use SignalTransition instead.")
};

#endif // STATEMACHINEFOREIGN_H
