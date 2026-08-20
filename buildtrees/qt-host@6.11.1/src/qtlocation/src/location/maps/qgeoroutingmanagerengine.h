// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTINGMANAGERENGINE_H
#define QGEOROUTINGMANAGERENGINE_H

#include <QtCore/QObject>
#include <QtCore/QLocale>
#include <QtLocation/QGeoRouteRequest>
#include <QtLocation/QGeoRouteReply>

QT_BEGIN_NAMESPACE

class QGeoCoordinate;
class QGeoRoutingManagerEnginePrivate;

class Q_LOCATION_EXPORT QGeoRoutingManagerEngine : public QObject
{
    Q_OBJECT
public:
    explicit QGeoRoutingManagerEngine(const QVariantMap &parameters, QObject *parent = nullptr);
    virtual ~QGeoRoutingManagerEngine();

    QString managerName() const;
    int managerVersion() const;

    virtual QGeoRouteReply *calculateRoute(const QGeoRouteRequest &request) = 0;
    virtual QGeoRouteReply *updateRoute(const QGeoRoute &route, const QGeoCoordinate &position);

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

protected:
    void setSupportedTravelModes(QGeoRouteRequest::TravelModes travelModes);
    void setSupportedFeatureTypes(QGeoRouteRequest::FeatureTypes featureTypes);
    void setSupportedFeatureWeights(QGeoRouteRequest::FeatureWeights featureWeights);
    void setSupportedRouteOptimizations(QGeoRouteRequest::RouteOptimizations optimizations);
    void setSupportedSegmentDetails(QGeoRouteRequest::SegmentDetails segmentDetails);
    void setSupportedManeuverDetails(QGeoRouteRequest::ManeuverDetails maneuverDetails);

private:
    void setManagerName(const QString &managerName);
    void setManagerVersion(int managerVersion);

    QGeoRoutingManagerEnginePrivate *d_ptr;
    Q_DISABLE_COPY(QGeoRoutingManagerEngine)

    friend class QGeoServiceProvider;
    friend class QGeoServiceProviderPrivate;
};

QT_END_NAMESPACE

#endif
