// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qgeoserviceproviderplugin_test.h"
#include "qgeocodingmanagerengine_test.h"

#include <QtPlugin>

QGeoServiceProviderFactoryTest::QGeoServiceProviderFactoryTest()
{
}

QGeoServiceProviderFactoryTest::~QGeoServiceProviderFactoryTest()
{
}

QGeoCodingManagerEngine* QGeoServiceProviderFactoryTest::createGeocodingManagerEngine(
                const QVariantMap &parameters, QGeoServiceProvider::Error *error,
                QString *errorString) const
{
    return new QGeoCodingManagerEngineTest(parameters, error, errorString);
}
