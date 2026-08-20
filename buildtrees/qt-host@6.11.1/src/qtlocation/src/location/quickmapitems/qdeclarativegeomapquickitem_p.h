// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOMAPQUICKITEM_H
#define QDECLARATIVEGEOMAPQUICKITEM_H

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

#include <QtQuick/QQuickItem>
#include <QtQuick/QSGNode>

#include <QtLocation/private/qdeclarativegeomap_p.h>
#include <QtLocation/private/qdeclarativegeomapitembase_p.h>
#include <QtPositioning/qgeoshape.h>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QMapQuickItemMatrix4x4 : public QQuickTransform
{
public:
    QMapQuickItemMatrix4x4(QObject *parent = nullptr);

    void setMatrix(const QMatrix4x4& matrix);
    void applyTo(QMatrix4x4 *matrix) const override;

    QMatrix4x4 m_matrix;
};

class Q_LOCATION_EXPORT QDeclarativeGeoMapQuickItem : public QDeclarativeGeoMapItemBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MapQuickItem)
    QML_ADDED_IN_VERSION(5, 0)
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate NOTIFY coordinateChanged)
    Q_PROPERTY(QPointF anchorPoint READ anchorPoint WRITE setAnchorPoint NOTIFY anchorPointChanged)
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(QQuickItem *sourceItem READ sourceItem WRITE setSourceItem NOTIFY sourceItemChanged)

public:
    explicit QDeclarativeGeoMapQuickItem(QQuickItem *parent = nullptr);
    ~QDeclarativeGeoMapQuickItem();

    void setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map) override;

    void setCoordinate(const QGeoCoordinate &coordinate);
    QGeoCoordinate coordinate();

    void setSourceItem(QQuickItem *sourceItem);
    QQuickItem *sourceItem();

    void setAnchorPoint(const QPointF &anchorPoint);
    QPointF anchorPoint() const;

    void setZoomLevel(qreal zoomLevel);
    qreal zoomLevel() const;

    const QGeoShape &geoShape() const override;
    void setGeoShape(const QGeoShape &shape) override;

    bool contains(const QPointF &point) const override;

Q_SIGNALS:
    void coordinateChanged();
    void sourceItemChanged();
    void anchorPointChanged();
    void zoomLevelChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void updatePolish() override;

protected Q_SLOTS:
    void afterChildrenChanged() override;
    void afterViewportChanged(const QGeoMapViewportChangeEvent &event) override;

private:
    qreal scaleFactor();
    QGeoCoordinate coordinate_;
    QGeoRectangle geoshape_;
    QPointer<QQuickItem> sourceItem_;
    QQuickItem *opacityContainer_ = nullptr;
    QPointF anchorPoint_;
    qreal zoomLevel_ = 0.0;
    bool mapAndSourceItemSet_ = false;
    bool updatingGeometry_ = false;
    QMapQuickItemMatrix4x4 *matrix_ = nullptr;

    friend class QDeclarativeGeoMap;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeGeoMapQuickItem)

#endif
