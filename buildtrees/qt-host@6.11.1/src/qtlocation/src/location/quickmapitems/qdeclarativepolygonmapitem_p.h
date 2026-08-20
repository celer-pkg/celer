// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEPOLYGONMAPITEM
#define QDECLARATIVEPOLYGONMAPITEM

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
#include <QtLocation/private/qdeclarativepolylinemapitem_p.h>
#include <QtPositioning/qgeopolygon.h>

QT_BEGIN_NAMESPACE

class QDeclarativePolygonMapItemPrivate;
class Q_LOCATION_EXPORT QDeclarativePolygonMapItem : public QDeclarativeGeoMapItemBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MapPolygon)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QList<QGeoCoordinate> path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(QDeclarativeMapLineProperties *border READ border CONSTANT)

public:
    explicit QDeclarativePolygonMapItem(QQuickItem *parent = nullptr);
    ~QDeclarativePolygonMapItem() override;

    void setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map) override;
    //from QuickItem
    QSGNode *updateMapItemPaintNode(QSGNode *, UpdatePaintNodeData *) override;

    Q_INVOKABLE void addCoordinate(const QGeoCoordinate &coordinate);
    Q_INVOKABLE void removeCoordinate(const QGeoCoordinate &coordinate);

    QList<QGeoCoordinate> path() const;
    void setPath(const QList<QGeoCoordinate> &value);

    QColor color() const;
    void setColor(const QColor &color);

    QDeclarativeMapLineProperties *border();

    bool contains(const QPointF &point) const override;
    const QGeoShape &geoShape() const override;
    void setGeoShape(const QGeoShape &shape) override;

Q_SIGNALS:
    void pathChanged();
    void colorChanged(const QColor &color);

protected Q_SLOTS:
    void markSourceDirtyAndUpdate();
    void onLinePropertiesChanged();
    void afterViewportChanged(const QGeoMapViewportChangeEvent &event) override;

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void updatePolish() override;

#ifdef QT_LOCATION_DEBUG
public:
#endif
    QGeoPolygon m_geopoly;
    QDeclarativeMapLineProperties m_border;
    QColor m_color;
    bool m_updatingGeometry;

    std::unique_ptr<QDeclarativePolygonMapItemPrivate> m_d;

    friend class QDeclarativePolygonMapItemPrivate;
    friend class QDeclarativePolygonMapItemPrivateCPU;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePolygonMapItem)

#endif /* QDECLARATIVEPOLYGONMAPITEM_H_ */
