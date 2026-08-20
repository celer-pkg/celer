// Copyright (C) 2020 Paolo Angelelli <paolo.angelelli@gmail.com>
// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVERECTANGLEMAPITEM_P_P_H
#define QDECLARATIVERECTANGLEMAPITEM_P_P_H

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
#include <QtLocation/private/qdeclarativerectanglemapitem_p.h>
#include <QtPositioning/private/qwebmercator_p.h>

QT_BEGIN_NAMESPACE

class QQuickShape;
class QQuickShapePath;

class Q_LOCATION_EXPORT QDeclarativeRectangleMapItemPrivate
{
    Q_DISABLE_COPY_MOVE(QDeclarativeRectangleMapItemPrivate)
public:
    QDeclarativeRectangleMapItemPrivate(QDeclarativeRectangleMapItem &rect);

    virtual ~QDeclarativeRectangleMapItemPrivate();
    virtual void onLinePropertiesChanged() = 0;
    virtual void markSourceDirtyAndUpdate() = 0;
    virtual void onMapSet() = 0;
    virtual void onGeoGeometryChanged() = 0;
    virtual void onItemGeometryChanged() = 0;
    virtual void updatePolish() = 0;
    virtual void afterViewportChanged() = 0;
    virtual QSGNode * updateMapItemPaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *data) = 0;
    virtual bool contains(const QPointF &point) const = 0;

    QDeclarativeRectangleMapItem &m_rect;
};

class Q_LOCATION_EXPORT QDeclarativeRectangleMapItemPrivateCPU: public QDeclarativeRectangleMapItemPrivate
{
public:
    QDeclarativeRectangleMapItemPrivateCPU(QDeclarativeRectangleMapItem &rect);
    ~QDeclarativeRectangleMapItemPrivateCPU() override;

    void onLinePropertiesChanged() override
    {
        // mark dirty just in case we're a width change
        markSourceDirtyAndUpdate();
    }
    void markSourceDirtyAndUpdate() override
    {
        m_geometry.markSourceDirty();
        m_rect.polishAndUpdate();
    }
    void onMapSet() override
    {
        markSourceDirtyAndUpdate();
    }
    void onGeoGeometryChanged() override
    {
        markSourceDirtyAndUpdate();
    }
    void onItemGeometryChanged() override
    {
        markSourceDirtyAndUpdate();
    }
    void afterViewportChanged() override
    {
        markSourceDirtyAndUpdate();
    }
    void updatePolish() override;
    QSGNode * updateMapItemPaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *data) override;
    bool contains(const QPointF &point) const override;

    QGeoMapPolygonGeometry m_geometry;
    QQuickShape *m_shape = nullptr;
    QQuickShapePath *m_shapePath = nullptr;
    QDeclarativeGeoMapPainterPath *m_painterPath = nullptr;
};

QT_END_NAMESPACE

#endif // QDECLARATIVERECTANGLEMAPITEM_P_P_H

