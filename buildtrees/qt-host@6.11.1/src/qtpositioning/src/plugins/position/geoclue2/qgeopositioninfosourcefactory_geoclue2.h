// Copyright (C) 2018 Denis Shienkov <denis.shienkov@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOPOSITIONINFOSOURCEFACTORY_GEOCLUE2_H
#define QGEOPOSITIONINFOSOURCEFACTORY_GEOCLUE2_H

#include <QtCore/QObject>
#include <QtPositioning/QGeoPositionInfoSourceFactory>

QT_BEGIN_NAMESPACE

/*
    Qt Positioning plugin for Geoclue. This plugin supports Geoclue version 2.x.
*/
class QGeoPositionInfoSourceFactoryGeoclue2 : public QObject, public QGeoPositionInfoSourceFactory
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

#endif // QGEOPOSITIONINFOSOURCEFACTORY_GEOCLUE2_H
