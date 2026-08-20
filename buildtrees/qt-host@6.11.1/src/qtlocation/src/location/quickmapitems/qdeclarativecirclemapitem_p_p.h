// Copyright (C) 2020 Paolo Angelelli <paolo.angelelli@gmail.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVECIRCLEMAPITEM_P_P_H
#define QDECLARATIVECIRCLEMAPITEM_P_P_H

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

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qdeclarativepolygonmapitem_p_p.h>
#include <QtLocation/private/qdeclarativecirclemapitem_p.h>

QT_BEGIN_NAMESPACE

class QQuickShape;
class QQuickShapePath;

class Q_LOCATION_EXPORT QGeoMapCircleGeometry : public QGeoMapPolygonGeometry
{
public:
    QGeoMapCircleGeometry();

    void updateSourceAndScreenPointsInvert(const QList<QDoubleVector2D> &circlePath, const QGeoMap &map);
};

class Q_LOCATION_EXPORT QDeclarativeCircleMapItemPrivate
{
    Q_DISABLE_COPY_MOVE(QDeclarativeCircleMapItemPrivate)
public:
    static const int CircleSamples = 128; // ToDo: make this radius && ZL dependent?

    QDeclarativeCircleMapItemPrivate(QDeclarativeCircleMapItem &circle)
        : m_circle(circle)
    {
    }

    virtual ~QDeclarativeCircleMapItemPrivate();
    virtual void onLinePropertiesChanged() = 0;
    virtual void markSourceDirtyAndUpdate() = 0;
    virtual void onMapSet() = 0;
    virtual void onGeoGeometryChanged() = 0;
    virtual void onItemGeometryChanged() = 0;
    virtual void updatePolish() = 0;
    virtual void afterViewportChanged() = 0;
    virtual QSGNode * updateMapItemPaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *data) = 0;
    virtual bool contains(const QPointF &point) const = 0;

    void updateCirclePath()
    {
        if (!m_circle.map() || m_circle.map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
            return;

        m_circlePath.clear();
        const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_circle.map()->geoProjection());
        if (m_circle.referenceSurface() == QLocation::ReferenceSurface::Map)
            calculatePeripheralPointsSimple(m_circlePath, m_circle.center(), m_circle.radius(), p, CircleSamples);
        else
            calculatePeripheralPointsGreatCircle(m_circlePath, m_circle.center(), m_circle.radius(), p, CircleSamples);
    }

    static int crossEarthPole(const QGeoCoordinate &center, qreal distance);

    static void includeOnePoleInPath(QList<QDoubleVector2D> &path, const QGeoCoordinate &center,
                                      qreal distance, const QGeoProjectionWebMercator &p);

    static void calculatePeripheralPointsSimple(QList<QDoubleVector2D> &path, const QGeoCoordinate &center,
                                   qreal distance, const QGeoProjectionWebMercator &p, int steps);

    static void calculatePeripheralPointsGreatCircle(QList<QDoubleVector2D> &path, const QGeoCoordinate &center,
                                   qreal distance, const QGeoProjectionWebMercator &p, int steps);

    QDeclarativeCircleMapItem &m_circle;
    QList<QDoubleVector2D> m_circlePath;
};

class Q_LOCATION_EXPORT QDeclarativeCircleMapItemPrivateCPU: public QDeclarativeCircleMapItemPrivate
{
public:
    QDeclarativeCircleMapItemPrivateCPU(QDeclarativeCircleMapItem &circle);
    ~QDeclarativeCircleMapItemPrivateCPU() override;

    void onLinePropertiesChanged() override
    {
        // mark dirty just in case we're a width change
        markSourceDirtyAndUpdate();
    }
    void markSourceDirtyAndUpdate() override
    {
        m_geometry.markSourceDirty();
        m_circle.polishAndUpdate();
    }
    void onMapSet() override
    {
        updateCirclePath();
        markSourceDirtyAndUpdate();
    }
    void onGeoGeometryChanged() override
    {
        updateCirclePath();
        markSourceDirtyAndUpdate();
    }
    void onItemGeometryChanged() override
    {
        onGeoGeometryChanged();
    }
    void afterViewportChanged() override
    {
        markSourceDirtyAndUpdate();
    }
    void updatePolish() override;
    QSGNode * updateMapItemPaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *data) override;
    bool contains(const QPointF &point) const override;

    QGeoMapCircleGeometry m_geometry;
    QQuickShape *m_shape = nullptr;
    QQuickShapePath *m_shapePath = nullptr;
    QDeclarativeGeoMapPainterPath *m_painterPath = nullptr;
};

QT_END_NAMESPACE

#endif // QDECLARATIVECIRCLEMAPITEM_P_P_H
