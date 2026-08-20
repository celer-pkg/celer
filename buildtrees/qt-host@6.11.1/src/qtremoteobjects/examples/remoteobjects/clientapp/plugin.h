// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QObject>
#include <QQmlEngine>
#include "rep_timemodel_replica.h"

// A "TimeModel" class with hour and minute properties
// that change on-the-minute yet efficiently sleep the rest
// of the time.

struct TimeModel
{
    Q_GADGET
    QML_FOREIGN(MinuteTimerReplica)
    QML_NAMED_ELEMENT(Time)
};
