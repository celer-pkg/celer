// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qgeotiledmap_test.h"
#include <QtCore/QMetaProperty>
#include <QtPositioning/QGeoCoordinate>
#include <QtLocation/private/qgeotiledmap_p_p.h>

QT_USE_NAMESPACE

class QGeoTiledMapTestPrivate: public QGeoTiledMapPrivate
{
    Q_DECLARE_PUBLIC(QGeoTiledMapTest)
public:
    QGeoTiledMapTestPrivate(QGeoTiledMappingManagerEngine *engine,
                            const QGeoTiledMapTestOptions &options)
        : QGeoTiledMapPrivate(engine), m_options(options)
    {

    }

    ~QGeoTiledMapTestPrivate()
    {

    }

    void setVisibleArea(const QRectF &visibleArea) override
    {
        if (m_options.supportVisibleArea)
            return QGeoTiledMapPrivate::setVisibleArea(visibleArea);
        else
            return QGeoMapPrivate::setVisibleArea(visibleArea);
    }

    QRectF visibleArea() const override
    {
        if (m_options.supportVisibleArea)
            return QGeoTiledMapPrivate::visibleArea();
        else
            return QGeoMapPrivate::visibleArea();
    }

    QGeoTiledMapTestOptions m_options;
};

QGeoTiledMapTest::QGeoTiledMapTest(QGeoTiledMappingManagerEngine *engine,
                                   const QGeoTiledMapTestOptions &options,
                                   QObject *parent)
:   QGeoTiledMap(*new QGeoTiledMapTestPrivate(engine, options), engine, parent), m_engine(engine)
{
}
