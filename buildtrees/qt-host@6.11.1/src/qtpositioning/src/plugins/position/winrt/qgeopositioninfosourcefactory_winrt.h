// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOPOSITIONINFOSOURCEFACTORY_WINRT_H
#define QGEOPOSITIONINFOSOURCEFACTORY_WINRT_H

#include <QtCore/qobject.h>
#include <QtPositioning/qgeopositioninfosourcefactory.h>

QT_BEGIN_NAMESPACE

class QGeoPositionInfoSourceFactoryWinRT : public QObject, public QGeoPositionInfoSourceFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.position.sourcefactory/6.0"
                      FILE "plugin.json")
    Q_INTERFACES(QGeoPositionInfoSourceFactory)
public:
    QGeoPositionInfoSource *positionInfoSource(QObject *parent, const QVariantMap &parameters) override;
    QGeoSatelliteInfoSource *satelliteInfoSource(QObject *parent, const QVariantMap &parameters) override;
    QGeoAreaMonitorSource *areaMonitor(QObject *parent, const QVariantMap &parameters) override;
};

QT_END_NAMESPACE

#endif // QGEOPOSITIONINFOSOURCEFACTORY_WINRT_H
