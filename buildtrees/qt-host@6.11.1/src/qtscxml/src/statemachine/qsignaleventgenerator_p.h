// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#ifndef QSIGNALEVENTGENERATOR_P_H
#define QSIGNALEVENTGENERATOR_P_H

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

#include <QtCore/private/qglobal_p.h>
#include <QtCore/qobject.h>

#include <private/qstatemachineglobal_p.h>

QT_REQUIRE_CONFIG(statemachine);

QT_BEGIN_NAMESPACE

class QStateMachine;

class QSignalEventGenerator : public QObject
{
    Q_OBJECT
public:
    QSignalEventGenerator(QStateMachine *parent);

private Q_SLOTS:
    void execute(QMethodRawArguments a);

private:
    Q_DISABLE_COPY_MOVE(QSignalEventGenerator)
};

QT_END_NAMESPACE

#endif
