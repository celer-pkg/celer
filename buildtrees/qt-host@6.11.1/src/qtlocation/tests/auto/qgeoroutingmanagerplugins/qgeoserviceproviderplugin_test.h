// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGEOSERVICEPROVIDER_TEST_H
#define QGEOSERVICEPROVIDER_TEST_H

#include <qgeoserviceproviderfactory.h>
#include <QObject>

QT_USE_NAMESPACE

class QGeoServiceProviderFactoryTest: public QObject, public QGeoServiceProviderFactory
{
    Q_OBJECT
    Q_INTERFACES(QGeoServiceProviderFactory)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.geoservice.serviceproviderfactory/6.0"
                      FILE "routing_plugin.json")

public:
    QGeoServiceProviderFactoryTest();
    ~QGeoServiceProviderFactoryTest();

    QGeoRoutingManagerEngine* createRoutingManagerEngine(const QVariantMap &parameters,
                                                         QGeoServiceProvider::Error *error,
                                                         QString *errorString) const override;

};

#endif


