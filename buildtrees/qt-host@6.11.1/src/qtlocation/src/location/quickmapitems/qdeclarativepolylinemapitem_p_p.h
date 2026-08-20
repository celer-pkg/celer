// Copyright (C) 2020 Paolo Angelelli <paolo.angelelli@gmail.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEPOLYLINEMAPITEM_P_P_H
#define QDECLARATIVEPOLYLINEMAPITEM_P_P_H

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

#include <QtCore/QScopedValueRollback>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtLocation/private/qgeomapitemgeometry_p.h>

#include <QtPositioning/private/qdoublevector2d_p.h>

QT_BEGIN_NAMESPACE

class QQuickShape;
class QQuickShapePath;

struct QGeoMapPolylineGeometry : QGeoMapItemGeometry
{
    void updateSourcePoints(const QGeoMap &map,
                            const QList<QDoubleVector2D> &basePath);

    QPainterPath srcPath() const { return srcPath_; }

    QPainterPath srcPath_;
    qreal maxCoord_ = 0.0;
};

class Q_LOCATION_EXPORT QDeclarativePolylineMapItemPrivate
{
    Q_DISABLE_COPY_MOVE(QDeclarativePolylineMapItemPrivate)
public:
    QDeclarativePolylineMapItemPrivate(QDeclarativePolylineMapItem &poly)
        : m_poly(poly)
    {
    }

    virtual ~QDeclarativePolylineMapItemPrivate();
    virtual void markSourceDirtyAndUpdate() = 0;
    virtual void onMapSet() = 0;
    virtual void onLinePropertiesChanged() = 0;
    virtual void onGeoGeometryChanged() = 0;
    virtual void onGeoGeometryUpdated() = 0;
    virtual void onItemGeometryChanged() = 0;
    virtual void updatePolish() = 0;
    virtual void afterViewportChanged() = 0;
    virtual QSGNode * updateMapItemPaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *data) = 0;
    virtual bool contains(const QPointF &point) const = 0;

    QDeclarativePolylineMapItem &m_poly;
};

class Q_LOCATION_EXPORT QDeclarativePolylineMapItemPrivateCPU: public QDeclarativePolylineMapItemPrivate
{
public:
    QDeclarativePolylineMapItemPrivateCPU(QDeclarativePolylineMapItem &poly);
    ~QDeclarativePolylineMapItemPrivateCPU() override;

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
    void regenerateCache();
    void updateCache();
    void afterViewportChanged() override
    {
        // preserveGeometry is cleared in updateMapItemPaintNode
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
    QSGNode *updateMapItemPaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData * /*data*/) override;
    bool contains(const QPointF &point) const override;

    QList<QDoubleVector2D> m_geopathProjected;
    QGeoMapPolylineGeometry m_geometry;
    QQuickShape *m_shape = nullptr;
    QQuickShapePath *m_shapePath = nullptr;
    QDeclarativeGeoMapPainterPath *m_painterPath = nullptr;
};

QT_END_NAMESPACE

#endif // QDECLARATIVEPOLYLINEMAPITEM_P_P_H
