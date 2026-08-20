// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTINGMANAGER_H
#define QGEOROUTINGMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QLocale>
#include <QtLocation/QGeoRouteRequest>
#include <QtLocation/QGeoRouteReply>

QT_BEGIN_NAMESPACE

class QGeoRoutingManagerEngine;
class QGeoRoutingManagerPrivate;

class Q_LOCATION_EXPORT QGeoRoutingManager : public QObject
{
    Q_OBJECT

public:
    ~QGeoRoutingManager();

    QString managerName() const;
    int managerVersion() const;

    QGeoRouteReply *calculateRoute(const QGeoRouteRequest &request);
    QGeoRouteReply *updateRoute(const QGeoRoute &route, const QGeoCoordinate &position);

    QGeoRouteRequest::TravelModes supportedTravelModes() const;
    QGeoRouteRequest::FeatureTypes supportedFeatureTypes() const;
    QGeoRouteRequest::FeatureWeights supportedFeatureWeights() const;
    QGeoRouteRequest::RouteOptimizations supportedRouteOptimizations() const;
    QGeoRouteRequest::SegmentDetails supportedSegmentDetails() const;
    QGeoRouteRequest::ManeuverDetails supportedManeuverDetails() const;

    void setLocale(const QLocale &locale);
    QLocale locale() const;
    void setMeasurementSystem(QLocale::MeasurementSystem system);
    QLocale::MeasurementSystem measurementSystem() const;

Q_SIGNALS:
    void finished(QGeoRouteReply *reply);
    void errorOccurred(QGeoRouteReply *reply, QGeoRouteReply::Error error,
                       const QString &errorString = QString());

private:
    explicit QGeoRoutingManager(QGeoRoutingManagerEngine *engine, QObject *parent = nullptr);

    QGeoRoutingManagerPrivate *d_ptr;
    Q_DISABLE_COPY(QGeoRoutingManager)

    friend class QGeoServiceProvider;
    friend class QGeoServiceProviderPrivate;
};

QT_END_NAMESPACE

#endif
