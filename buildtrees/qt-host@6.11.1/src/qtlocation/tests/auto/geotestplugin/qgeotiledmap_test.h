// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QGEOTILEDMAP_TEST_H
#define QGEOTILEDMAP_TEST_H

#include <QtLocation/private/qgeotiledmappingmanagerengine_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>

QT_USE_NAMESPACE

class QGeoTiledMappingManagerEngineTest;
class QGeoTiledMapTestPrivate;

struct QGeoTiledMapTestOptions {
    bool supportVisibleArea = true;
};

class QGeoTiledMapTest: public QGeoTiledMap
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoTiledMapTest)
public:
    QGeoTiledMapTest(QGeoTiledMappingManagerEngine *engine,
                     const QGeoTiledMapTestOptions &options,
                     QObject *parent = nullptr);

public:
    using QGeoTiledMap::setCameraData;
    QGeoTiledMappingManagerEngine *m_engine;
};

#endif


