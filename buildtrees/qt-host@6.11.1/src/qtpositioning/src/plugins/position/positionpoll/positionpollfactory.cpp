// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "positionpollfactory.h"
#include "qgeoareamonitor_polling.h"

QT_BEGIN_NAMESPACE

QGeoPositionInfoSource *QGeoPositionInfoSourceFactoryPoll::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}

QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactoryPoll::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}

QGeoAreaMonitorSource *QGeoPositionInfoSourceFactoryPoll::areaMonitor(QObject *parent, const QVariantMap &parameters)
{
    Q_UNUSED(parameters)
    QGeoAreaMonitorPolling *ret = new QGeoAreaMonitorPolling(parent);
    if (ret && ret->isValid())
        return ret;
    delete ret;
    return nullptr;
}

QT_END_NAMESPACE

#include "moc_positionpollfactory.cpp"
