// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include "qqmlprofilereventreceiver_p.h"

QT_BEGIN_NAMESPACE

QQmlProfilerEventReceiver::~QQmlProfilerEventReceiver() = default;

bool QQmlProfilerEventReceiver::save(const QString &filename)
{
    Q_UNUSED(filename);
    return false;
}

void QQmlProfilerEventReceiver::startTrace(qint64 time, const QList<int> &engineIds)
{
    Q_UNUSED(time);
    Q_UNUSED(engineIds);
}

void QQmlProfilerEventReceiver::endTrace(qint64 time, const QList<int> &engineIds)
{
    Q_UNUSED(time);
    Q_UNUSED(engineIds);
}

void QQmlProfilerEventReceiver::complete(qint64 maximumTime)
{
    Q_UNUSED(maximumTime);
    emit dataReady();
}

QQmlProfilerEventReceiver::QQmlProfilerEventReceiver(QObjectPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
}

QT_END_NAMESPACE

#include "moc_qqmlprofilereventreceiver_p.cpp"
