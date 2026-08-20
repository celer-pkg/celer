// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qgeoserviceproviderplugin_test.h"
#include "qgeoroutingmanagerengine_test.h"

#include <QtPlugin>

QGeoServiceProviderFactoryTest::QGeoServiceProviderFactoryTest()
{
}

QGeoServiceProviderFactoryTest::~QGeoServiceProviderFactoryTest()
{
}

QGeoRoutingManagerEngine* QGeoServiceProviderFactoryTest::createRoutingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error,
    QString *errorString) const
{
    return new QGeoRoutingManagerEngineTest(parameters, error, errorString);
}
