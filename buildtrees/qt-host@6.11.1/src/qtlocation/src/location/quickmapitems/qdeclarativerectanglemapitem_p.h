// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVERECTANGLEMAPITEM_H_
#define QDECLARATIVERECTANGLEMAPITEM_H_

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
#include <QtLocation/private/qdeclarativegeomapitembase_p.h>
#include <QtLocation/private/qgeomapitemgeometry_p.h>
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtLocation/private/qdeclarativepolygonmapitem_p_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>

#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>

QT_BEGIN_NAMESPACE

class QDeclarativeRectangleMapItemPrivate;
class Q_LOCATION_EXPORT QDeclarativeRectangleMapItem: public QDeclarativeGeoMapItemBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MapRectangle)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QGeoCoordinate topLeft READ topLeft WRITE setTopLeft NOTIFY topLeftChanged)
    Q_PROPERTY(QGeoCoordinate bottomRight READ bottomRight WRITE setBottomRight NOTIFY bottomRightChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QDeclarativeMapLineProperties *border READ border CONSTANT)

public:
    explicit QDeclarativeRectangleMapItem(QQuickItem *parent = nullptr);
    ~QDeclarativeRectangleMapItem() override;

    void setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map) override;
    //from QuickItem
    QSGNode *updateMapItemPaintNode(QSGNode *, UpdatePaintNodeData *) override;

    QGeoCoordinate topLeft();
    void setTopLeft(const QGeoCoordinate &center);

    QGeoCoordinate bottomRight();
    void setBottomRight(const QGeoCoordinate &center);

    QColor color() const;
    void setColor(const QColor &color);

    QDeclarativeMapLineProperties *border();

    bool contains(const QPointF &point) const override;
    const QGeoShape &geoShape() const override;
    void setGeoShape(const QGeoShape &shape) override;

Q_SIGNALS:
    void topLeftChanged(const QGeoCoordinate &topLeft);
    void bottomRightChanged(const QGeoCoordinate &bottomRight);
    void colorChanged(const QColor &color);

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void updatePolish() override;

protected Q_SLOTS:
    void markSourceDirtyAndUpdate();
    void onLinePropertiesChanged();
    void afterViewportChanged(const QGeoMapViewportChangeEvent &event) override;

private:
    QGeoRectangle m_rectangle;
    QDeclarativeMapLineProperties m_border;
    QColor m_color = Qt::transparent;

    bool m_updatingGeometry = false;

    std::unique_ptr<QDeclarativeRectangleMapItemPrivate> m_d;

    friend class QDeclarativeRectangleMapItemPrivate;
    friend class QDeclarativeRectangleMapItemPrivateCPU;
};

//////////////////////////////////////////////////////////////////////

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeRectangleMapItem)

#endif /* QDECLARATIVERECTANGLEMAPITEM_H_ */
