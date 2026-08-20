// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeopositioninfosourcefactory.h"

QT_BEGIN_NAMESPACE

/*!
  \class QGeoPositionInfoSourceFactory
  \inmodule QtPositioning
  \since 5.2

  \brief The QGeoPositionInfoSourceFactory class is a factory class used
  as the plugin interface for external providers of positioning data.

  Each factory method takes a parameters argument, which allows to configure
  the created source.
*/

/*!
  \fn QGeoPositionInfoSource *QGeoPositionInfoSourceFactory::positionInfoSource(QObject *parent, const QVariantMap &parameters)

  Returns a new QGeoPositionInfoSource associated with this plugin
  with parent \a parent, and using \a parameters as configuration parameters.
  Can also return 0, in which case the plugin loader will use the factory with
  the next highest priority.
*/

/*!
  \fn QGeoSatelliteInfoSource *QGeoPositionInfoSourceFactory::satelliteInfoSource(QObject *parent, const QVariantMap &parameters)

  Returns a new QGeoSatelliteInfoSource associated with this plugin
  with parent \a parent, and using \a parameters as configuration parameters.
  Can also return 0, in which case the plugin loader will use the factory with
  the next highest priority.
*/

/*!
  \fn QGeoAreaMonitorSource *QGeoPositionInfoSourceFactory::areaMonitor(QObject *parent, const QVariantMap &parameters);

  Returns a new QGeoAreaMonitorSource associated with this plugin with parent
  \a parent, and using \a parameters as configuration parameters.
  Can also return 0, in which case the plugin loader will use the factory with
  the next highest priority.
*/

/*!
    Destroys the position info source factory.
*/
QGeoPositionInfoSourceFactory::~QGeoPositionInfoSourceFactory()
{}

QT_END_NAMESPACE

