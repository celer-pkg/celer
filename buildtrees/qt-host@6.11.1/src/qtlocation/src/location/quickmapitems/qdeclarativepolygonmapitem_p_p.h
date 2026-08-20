// Copyright (C) 2020 Paolo Angelelli <paolo.angelelli@gmail.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEPOLYGONMAPITEM_P_P_H
#define QDECLARATIVEPOLYGONMAPITEM_P_P_H

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

#include <QtCore/QList>
#include <QtCore/QScopedValueRollback>
#include <QtGui/QMatrix4x4>
#include <QtGui/QColor>

#include <QSGFlatColorMaterial>

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qgeomapitemgeometry_p.h>
#include <QtLocation/private/qdeclarativepolygonmapitem_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p_p.h>

#include <QtPositioning/private/qdoublevector2d_p.h>

QT_BEGIN_NAMESPACE

class QQuickShape;
class QQuickShapePath;

class Q_LOCATION_EXPORT QGeoMapPolygonGeometry : public QGeoMapItemGeometry
{
public:
    enum MapBorderBehaviour {
        DrawOnce,
        Duplicate,
        WrapAround
    };

    QGeoMapPolygonGeometry();

    inline void setAssumeSimple(bool value) { assumeSimple_ = value; }

    void updateSourcePoints(const QGeoMap &map,
                            const QList<QList<QDoubleVector2D>> &path,
                            MapBorderBehaviour wrapping = Duplicate);

    QPainterPath srcPath() const { return srcPath_; }
    qreal maxCoord() const { return maxCoord_; }

protected:
    QPainterPath srcPath_;
    qreal maxCoord_ = 0.0;
    bool assumeSimple_ = false;
};

class Q_LOCATION_EXPORT QDeclarativePolygonMapItemPrivate
{
    Q_DISABLE_COPY_MOVE(QDeclarativePolygonMapItemPrivate)
public:
    QDeclarativePolygonMapItemPrivate(QDeclarativePolygonMapItem &polygon)
        : m_poly(polygon)
    {
    }

    virtual ~QDeclarativePolygonMapItemPrivate();
    virtual void onLinePropertiesChanged() = 0;
    virtual void markSourceDirtyAndUpdate() = 0;
    virtual void onMapSet() = 0;
    virtual void onGeoGeometryChanged() = 0;
    virtual void onGeoGeometryUpdated() = 0;
    virtual void onItemGeometryChanged() = 0;
    virtual void updatePolish() = 0;
    virtual void afterViewportChanged() = 0;
    virtual QSGNode * updateMapItemPaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *data) = 0;
    virtual bool contains(const QPointF &point) const = 0;

    QDeclarativePolygonMapItem &m_poly;
};

class Q_LOCATION_EXPORT QDeclarativePolygonMapItemPrivateCPU: public QDeclarativePolygonMapItemPrivate
{
public:
    QDeclarativePolygonMapItemPrivateCPU(QDeclarativePolygonMapItem &polygon);
    ~QDeclarativePolygonMapItemPrivateCPU() override;

    void onLinePropertiesChanged() override
    {
        // mark dirty just in case we're a width change
        markSourceDirtyAndUpdate();
    }
    void markSourceDirtyAndUpdate() override
    {
        m_geometry.markSourceDirty();
        m_poly.polishAndUpdate();
    }
    void regenerateCache()
    {
        if (!m_poly.map() || m_poly.map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
            return;
        const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_poly.map()->geoProjection());
        m_geopathProjected.clear();
        m_geopathProjected << QList<QDoubleVector2D>();
        QList<QDoubleVector2D> &pP = m_geopathProjected.last();
        if (m_poly.referenceSurface() == QLocation::ReferenceSurface::Globe) {
            const QList<QGeoCoordinate> realPath = QDeclarativeGeoMapItemUtils::greaterCirclePath(m_poly.m_geopoly.perimeter(),
                                                                                            QDeclarativeGeoMapItemUtils::ClosedPath);
            pP.reserve(realPath.size());
            for (const QGeoCoordinate &c : realPath)
                pP << p.geoToMapProjection(c);
        } else {
            pP.reserve(m_poly.m_geopoly.perimeter().size());
            const QList<QGeoCoordinate> perimeter = m_poly.m_geopoly.perimeter();
            for (const QGeoCoordinate &c : perimeter)
                pP << p.geoToMapProjection(c);
        }
        for (int i = 0; i < m_poly.m_geopoly.holesCount(); i++) {
            m_geopathProjected << QList<QDoubleVector2D>();
            QList<QDoubleVector2D> &pH = m_geopathProjected.last();
            if (m_poly.referenceSurface() == QLocation::ReferenceSurface::Globe) {
                const QList<QGeoCoordinate> realPath = QDeclarativeGeoMapItemUtils::greaterCirclePath(m_poly.m_geopoly.holePath(i),
                                                                                                QDeclarativeGeoMapItemUtils::ClosedPath);
                pH.reserve(realPath.size());
                for (const QGeoCoordinate &c : realPath)
                    pH << p.geoToMapProjection(c);
            } else {
                pH.reserve(m_poly.m_geopoly.holePath(i).size());
                const QList<QGeoCoordinate> holePath = m_poly.m_geopoly.holePath(i);
                for (const QGeoCoordinate &c : holePath)
                    pH << p.geoToMapProjection(c);
            }
        }
    }
    void updateCache()
    {
        if (!m_poly.map() || m_poly.map()->geoProjection().projectionType() != QGeoProjection::ProjectionWebMercator)
            return;
        const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_poly.map()->geoProjection());
        QList<QDoubleVector2D> &pP = m_geopathProjected.first();
        if (m_poly.referenceSurface() == QLocation::ReferenceSurface::Globe && m_poly.m_geopoly.perimeter().size() > 1) {
            regenerateCache(); //giving up here. Too difficult to take back all the interpolated points
        } else {
            pP << p.geoToMapProjection(m_poly.m_geopoly.perimeter().last());
        }
    }
    void afterViewportChanged() override
    {
        markSourceDirtyAndUpdate();
    }
    void onMapSet() override
    {
        regenerateCache();
        markSourceDirtyAndUpdate();
    }
    void onGeoGeometryChanged() override
    {
        regenerateCache();
        markSourceDirtyAndUpdate();
    }
    void onGeoGeometryUpdated() override
    {
        updateCache();
        markSourceDirtyAndUpdate();
    }
    void onItemGeometryChanged() override
    {
        onGeoGeometryChanged();
    }
    void updatePolish() override;
    QSGNode *updateMapItemPaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *data) override;
    bool contains(const QPointF &point) const override;

    QList<QList<QDoubleVector2D>> m_geopathProjected;
    QGeoMapPolygonGeometry m_geometry;
    QQuickShape *m_shape = nullptr;
    QQuickShapePath *m_shapePath = nullptr;
    QDeclarativeGeoMapPainterPath *m_painterPath = nullptr;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEPOLYGONMAPITEM_P_P_H
