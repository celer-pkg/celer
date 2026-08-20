// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qgeoserviceproviderplugin_test.h"

#include <QtCore/QtPlugin>
#include <QtLocation/QPlaceManagerEngine>

QGeoServiceProviderFactoryTest::QGeoServiceProviderFactoryTest()
{
}

QGeoServiceProviderFactoryTest::~QGeoServiceProviderFactoryTest()
{
}

QPlaceManagerEngine *QGeoServiceProviderFactoryTest::createPlaceManagerEngine(
        const QVariantMap &parameters,
        QGeoServiceProvider::Error *error, QString *errorString) const
{
    Q_UNUSED(error);
    Q_UNUSED(errorString);

    return new QPlaceManagerEngine(parameters);
}
