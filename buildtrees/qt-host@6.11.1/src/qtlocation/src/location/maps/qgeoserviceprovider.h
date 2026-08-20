// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOSERVICEPROVIDER_H
#define QGEOSERVICEPROVIDER_H

#include <QtCore/QVariantMap>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtLocation/qlocationglobal.h>

QT_BEGIN_NAMESPACE

class QLocale;
class QGeoCodingManager;
class QGeoMappingManager;
class QGeoRoutingManager;
class QPlaceManager;
class QGeoCodingManagerEngine;
class QGeoMappingManagerEngine;
class QGeoRoutingManagerEngine;
class QPlaceManagerEngine;
class QGeoServiceProviderPrivate;
class QQmlEngine;

class Q_LOCATION_EXPORT QGeoServiceProvider : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError,
        NotSupportedError,
        UnknownParameterError,
        MissingRequiredParameterError,
        ConnectionError,
        LoaderError
    };
    Q_ENUM(Error)

    enum RoutingFeature {
        NoRoutingFeatures               = 0,
        OnlineRoutingFeature            = (1<<0),
        OfflineRoutingFeature           = (1<<1),
        LocalizedRoutingFeature         = (1<<2),
        RouteUpdatesFeature             = (1<<3),
        AlternativeRoutesFeature        = (1<<4),
        ExcludeAreasRoutingFeature      = (1<<5),
        AnyRoutingFeatures              = ~(0)
    };

    enum GeocodingFeature {
        NoGeocodingFeatures             = 0,
        OnlineGeocodingFeature          = (1<<0),
        OfflineGeocodingFeature         = (1<<1),
        ReverseGeocodingFeature         = (1<<2),
        LocalizedGeocodingFeature       = (1<<3),
        AnyGeocodingFeatures            = ~(0)
    };

    enum MappingFeature {
        NoMappingFeatures               = 0,
        OnlineMappingFeature            = (1<<0),
        OfflineMappingFeature           = (1<<1),
        LocalizedMappingFeature         = (1<<2),
        AnyMappingFeatures              = ~(0)
    };

    enum PlacesFeature {
        NoPlacesFeatures                = 0,
        OnlinePlacesFeature             = (1<<0),
        OfflinePlacesFeature            = (1<<1),
        SavePlaceFeature                = (1<<2),
        RemovePlaceFeature              = (1<<3),
        SaveCategoryFeature             = (1<<4),
        RemoveCategoryFeature           = (1<<5),
        PlaceRecommendationsFeature     = (1<<6),
        SearchSuggestionsFeature        = (1<<7),
        LocalizedPlacesFeature          = (1<<8),
        NotificationsFeature            = (1<<9),
        PlaceMatchingFeature            = (1<<10),
        AnyPlacesFeatures               = ~(0)
    };

    enum NavigationFeature {
        NoNavigationFeatures            = 0,
        OnlineNavigationFeature         = (1<<0),
        OfflineNavigationFeature        = (1<<1),
        AnyNavigationFeatures           = ~(0)
    };

    Q_DECLARE_FLAGS(RoutingFeatures, RoutingFeature)
    Q_FLAG(RoutingFeatures)

    Q_DECLARE_FLAGS(GeocodingFeatures, GeocodingFeature)
    Q_FLAG(GeocodingFeatures)

    Q_DECLARE_FLAGS(MappingFeatures, MappingFeature)
    Q_FLAG(MappingFeatures)

    Q_DECLARE_FLAGS(PlacesFeatures, PlacesFeature)
    Q_FLAG(PlacesFeatures)

    Q_DECLARE_FLAGS(NavigationFeatures, NavigationFeature)
    Q_FLAG(NavigationFeatures)

    static QStringList availableServiceProviders();
    QGeoServiceProvider(const QString &providerName,
                        const QVariantMap &parameters = QVariantMap(),
                        bool allowExperimental = false);

    ~QGeoServiceProvider();

    RoutingFeatures routingFeatures() const;
    GeocodingFeatures geocodingFeatures() const;
    MappingFeatures mappingFeatures() const;
    PlacesFeatures placesFeatures() const;
    NavigationFeatures navigationFeatures() const;

    QGeoCodingManager *geocodingManager() const;
    QGeoMappingManager *mappingManager() const;
    QGeoRoutingManager *routingManager() const;
    QPlaceManager *placeManager() const;

    Error error() const;
    QString errorString() const;

    Error mappingError() const;
    QString mappingErrorString() const;
    Error geocodingError() const;
    QString geocodingErrorString() const;
    Error routingError() const;
    QString routingErrorString() const;
    Error placesError() const;
    QString placesErrorString() const;
    Error navigationError() const;
    QString navigationErrorString() const;

    void setParameters(const QVariantMap &parameters);
    void setLocale(const QLocale &locale);
    void setAllowExperimental(bool allow);
    void setQmlEngine(QQmlEngine *engine);

private:
    QGeoServiceProviderPrivate *d_ptr;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoServiceProvider::RoutingFeatures)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoServiceProvider::GeocodingFeatures)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoServiceProvider::MappingFeatures)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoServiceProvider::PlacesFeatures)
Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoServiceProvider::NavigationFeatures)

QT_END_NAMESPACE

#endif
