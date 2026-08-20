// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOSERVICEPROVIDERFACTORY_H
#define QGEOSERVICEPROVIDERFACTORY_H

#include <QtLocation/QGeoServiceProvider>

#include <QtCore/QtPlugin>
#include <QtCore/QMap>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE
class QQmlEngine;

class Q_LOCATION_EXPORT QGeoServiceProviderFactory
{
public:
    virtual ~QGeoServiceProviderFactory() {}

    virtual QGeoCodingManagerEngine *createGeocodingManagerEngine(const QVariantMap &parameters,
            QGeoServiceProvider::Error *error,
            QString *errorString) const;
    virtual QGeoMappingManagerEngine *createMappingManagerEngine(const QVariantMap &parameters,
            QGeoServiceProvider::Error *error,
            QString *errorString) const;
    virtual QGeoRoutingManagerEngine *createRoutingManagerEngine(const QVariantMap &parameters,
            QGeoServiceProvider::Error *error,
            QString *errorString) const;
    virtual QPlaceManagerEngine *createPlaceManagerEngine(const QVariantMap &parameters,
            QGeoServiceProvider::Error *error,
            QString *errorString) const;

    virtual void setQmlEngine(QQmlEngine * engine);
};

Q_DECLARE_INTERFACE(QGeoServiceProviderFactory,
                    "org.qt-project.qt.geoservice.serviceproviderfactory/6.0")

QT_END_NAMESPACE

#endif
