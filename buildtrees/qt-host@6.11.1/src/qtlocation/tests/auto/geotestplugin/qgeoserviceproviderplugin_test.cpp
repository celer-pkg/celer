// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qgeoserviceproviderplugin_test.h"
#include "qgeocodingmanagerengine_test.h"
#include "qgeoroutingmanagerengine_test.h"
#include "qgeotiledmappingmanagerengine_test.h"
#include "qplacemanagerengine_test.h"

#include <QtPlugin>

namespace
{
    template<class EngineType>
    EngineType * createEngine(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
    {
        const QString failError = parameters.value(QStringLiteral("error")).toString();
        const QString failErrorString = parameters.value(QStringLiteral("errorString")).toString();

        if (!failError.isEmpty()) {
            *error = QGeoServiceProvider::Error(failError.toInt());
            *errorString = failErrorString;
            return 0;
        } else {
            return new EngineType(parameters, error, errorString);
        }
    }
}

QGeoServiceProviderFactoryTest::QGeoServiceProviderFactoryTest()
{
}

QGeoServiceProviderFactoryTest::~QGeoServiceProviderFactoryTest()
{
}

QGeoRoutingManagerEngine* QGeoServiceProviderFactoryTest::createRoutingManagerEngine(
            const QVariantMap &parameters,
            QGeoServiceProvider::Error *error, QString *errorString) const
{
    return createEngine<QGeoRoutingManagerEngineTest>(parameters, error, errorString);
}


QGeoCodingManagerEngine* QGeoServiceProviderFactoryTest::createGeocodingManagerEngine(
                const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                QString *errorString) const
{
    return createEngine<QGeoCodingManagerEngineTest>(parameters, error, errorString);
}


QGeoMappingManagerEngine* QGeoServiceProviderFactoryTest::createMappingManagerEngine(
            const QVariantMap &parameters,
            QGeoServiceProvider::Error *error, QString *errorString) const
{
    return createEngine<QGeoTiledMappingManagerEngineTest>(parameters, error, errorString);
}

QPlaceManagerEngine* QGeoServiceProviderFactoryTest::createPlaceManagerEngine(
        const QVariantMap &parameters,
        QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(error);
    Q_UNUSED(errorString);
    return new QPlaceManagerEngineTest(parameters);
}
