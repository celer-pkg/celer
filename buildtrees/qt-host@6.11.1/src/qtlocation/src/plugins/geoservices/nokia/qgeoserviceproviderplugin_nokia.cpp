// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeoserviceproviderplugin_nokia.h"

#include "qgeocodingmanagerengine_nokia.h"
#include "qgeoroutingmanagerengine_nokia.h"
#include "qgeotiledmappingmanagerengine_nokia.h"
#include "qplacemanagerengine_nokiav2.h"
#include "qgeointrinsicnetworkaccessmanager.h"
#include "qgeoerror_messages.h"

#include <QtPlugin>
#include <QNetworkProxy>
#include <QCoreApplication>

QT_BEGIN_NAMESPACE

namespace
{
    bool isValidParameter(const QString &param)
    {
        if (param.isEmpty())
            return false;

        if (param.length() > 512)
            return false;

        for (const auto &c : param) {
            if (!c.isLetterOrNumber() && c.toLatin1() != '%' && c.toLatin1() != '-' &&
                c.toLatin1() != '+' && c.toLatin1() != '_') {
                return false;
            }
        }
        return true;
    }

    QGeoNetworkAccessManager *tryGetNetworkAccessManager(const QVariantMap &parameters)
    {
        return static_cast<QGeoNetworkAccessManager *>(qvariant_cast<void *>(parameters.value(QStringLiteral("nam"))));
    }

    void checkUsageTerms(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
    {
        const QString apiKey = parameters.value(QStringLiteral("here.apiKey")).toString();

        if (isValidParameter(apiKey))
             return;
        else
            qWarning() << "Invalid here.apiKey";

        if (parameters.contains(QStringLiteral("apiKey")))
            qWarning() << QStringLiteral("Please prefix 'apiKey' with prefix 'here' (e.g.: 'here.apiKey')");

        *error = QGeoServiceProvider::MissingRequiredParameterError;
        *errorString = QCoreApplication::translate(NOKIA_PLUGIN_CONTEXT_NAME, MISSED_CREDENTIALS);
    }

    template<class TInstance>
    TInstance * CreateInstanceOf(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
    {
        checkUsageTerms(parameters, error, errorString);

        if (*error != QGeoServiceProvider::NoError)
            return nullptr;

        QGeoNetworkAccessManager *networkManager = tryGetNetworkAccessManager(parameters);
        if (!networkManager)
            networkManager = new QGeoIntrinsicNetworkAccessManager(parameters);

        return new TInstance(networkManager, parameters, error, errorString);
    }
}

QGeoServiceProviderFactoryNokia::QGeoServiceProviderFactoryNokia()
{
}

QGeoCodingManagerEngine *QGeoServiceProviderFactoryNokia::createGeocodingManagerEngine(
        const QVariantMap &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    return CreateInstanceOf<QGeoCodingManagerEngineNokia>(parameters, error, errorString);
}

QGeoMappingManagerEngine *QGeoServiceProviderFactoryNokia::createMappingManagerEngine(
        const QVariantMap &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    return CreateInstanceOf<QGeoTiledMappingManagerEngineNokia>(parameters, error, errorString);
}

QGeoRoutingManagerEngine *QGeoServiceProviderFactoryNokia::createRoutingManagerEngine(
        const QVariantMap &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    return CreateInstanceOf<QGeoRoutingManagerEngineNokia>(parameters, error, errorString);
}

QPlaceManagerEngine *QGeoServiceProviderFactoryNokia::createPlaceManagerEngine(
        const QVariantMap &parameters,
        QGeoServiceProvider::Error *error,
        QString *errorString) const
{
    return CreateInstanceOf<QPlaceManagerEngineNokiaV2>(parameters, error, errorString);
}

QT_END_NAMESPACE
