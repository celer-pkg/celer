// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOSERVICEPROVIDER_P_H
#define QGEOSERVICEPROVIDER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qgeoserviceprovider.h"

#include <QHash>
#include <QCborMap>
#include <QLocale>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QGeoCodingManager;
class QGeoRoutingManager;
class QGeoMappingManager;

class QGeoServiceProviderFactory;
class QQmlEngine;

class QGeoServiceProviderPrivate
{
public:
    QGeoServiceProviderPrivate();
    ~QGeoServiceProviderPrivate();

    void loadMeta();
    void loadPlugin(const QVariantMap &parameters);
    void unload();
    void filterParameterMap();

    /* helper templates for generating the feature and manager accessors */
    template <class Manager, class Engine>
    Manager *manager(QGeoServiceProvider::Error *error,
                     QString *errorString);
    template <class Flags>
    Flags features(const char *enumName) const;

    QGeoServiceProviderFactory *factory = nullptr;
    QCborMap metaData;

    QVariantMap parameterMap;
    QVariantMap cleanedParameterMap;

    bool experimental = false;

    std::unique_ptr<QGeoCodingManager> geocodingManager;
    std::unique_ptr<QGeoRoutingManager> routingManager;
    std::unique_ptr<QGeoMappingManager> mappingManager;
    std::unique_ptr<QPlaceManager> placeManager;
    QQmlEngine *qmlEngine = nullptr;

    QGeoServiceProvider::Error geocodeError = QGeoServiceProvider::NoError;
    QGeoServiceProvider::Error routingError = QGeoServiceProvider::NoError;
    QGeoServiceProvider::Error mappingError = QGeoServiceProvider::NoError;
    QGeoServiceProvider::Error placeError = QGeoServiceProvider::NoError;
    QGeoServiceProvider::Error navigationError = QGeoServiceProvider::NoError;

    QString geocodeErrorString;
    QString routingErrorString;
    QString mappingErrorString;
    QString placeErrorString;
    QString navigationErrorString;

    QGeoServiceProvider::Error error = QGeoServiceProvider::NoError;
    QString errorString;

    QString providerName;

    QLocale locale;
    bool localeSet = false;

    static QMultiHash<QString, QCborMap> plugins(bool reload = false);
    static void loadPluginMetadata(QMultiHash<QString, QCborMap> &list);
};

QT_END_NAMESPACE

#endif
