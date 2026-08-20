// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopositioninfosourcefactory_winrt.h"
#include "qgeopositioninfosource_winrt_p.h"

#include <QtCore/qloggingcategory.h>

Q_LOGGING_CATEGORY(lcPositioningWinRT, "qt.positioning.winrt")

QT_BEGIN_NAMESPACE

QGeoPositionInfoSource *QGeoPositionInfoSourceFactoryWinRT::positionInfoSource(QObject *parent, const QVariantMap &parameters)
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_UNUSED(parameters)
    QGeoPositionInfoSourceWinRT *src = new QGeoPositionInfoSourceWinRT(parent);
    return src;
}

QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactoryWinRT::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}

QGeoAreaMonitorSource *QGeoPositionInfoSourceFactoryWinRT::areaMonitor(QObject *parent, const QVariantMap &parameters)
{
    qCDebug(lcPositioningWinRT) << __FUNCTION__;
    Q_UNUSED(parent)
    Q_UNUSED(parameters)
    return nullptr;
}

QT_END_NAMESPACE
