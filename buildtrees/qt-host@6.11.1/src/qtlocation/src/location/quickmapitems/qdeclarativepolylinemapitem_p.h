// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEPOLYLINEMAPITEM
#define QDECLARATIVEPOLYLINEMAPITEM

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

#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <QtPositioning/QGeoPath>

#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qdeclarativegeomapitembase_p.h>
#include <QtLocation/private/qgeomapitemgeometry_p.h>

#include <QtPositioning/private/qdoublevector2d_p.h>

QT_BEGIN_NAMESPACE

class Q_LOCATION_EXPORT QDeclarativeMapLineProperties : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)

public:
    explicit QDeclarativeMapLineProperties(QObject *parent = nullptr);

    QColor color() const;
    void setColor(const QColor &color);

    qreal width() const;
    void setWidth(qreal width);

Q_SIGNALS:
    void widthChanged(qreal width);
    void colorChanged(const QColor &color);

private:
    qreal width_ = 1.0;
    QColor color_ = Qt::black;
};

class QDeclarativePolylineMapItemPrivate;
class Q_LOCATION_EXPORT QDeclarativePolylineMapItem : public QDeclarativeGeoMapItemBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MapPolyline)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QList<QGeoCoordinate> path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QDeclarativeMapLineProperties *line READ line CONSTANT)

public:
    explicit QDeclarativePolylineMapItem(QQuickItem *parent = nullptr);
    ~QDeclarativePolylineMapItem();

    void setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map) override;
       //from QuickItem
    QSGNode *updateMapItemPaintNode(QSGNode *, UpdatePaintNodeData *) override;

    Q_INVOKABLE int pathLength() const;
    Q_INVOKABLE void addCoordinate(const QGeoCoordinate &coordinate);
    Q_INVOKABLE void insertCoordinate(int index, const QGeoCoordinate &coordinate);
    Q_INVOKABLE void replaceCoordinate(int index, const QGeoCoordinate &coordinate);
    Q_INVOKABLE QGeoCoordinate coordinateAt(int index) const;
    Q_INVOKABLE bool containsCoordinate(const QGeoCoordinate &coordinate);
    Q_INVOKABLE void removeCoordinate(const QGeoCoordinate &coordinate);
    Q_INVOKABLE void removeCoordinate(int index);

    QList<QGeoCoordinate> path() const;
    virtual void setPath(const QList<QGeoCoordinate> &value);
    Q_INVOKABLE void setPath(const QGeoPath &path);

    bool contains(const QPointF &point) const override;
    const QGeoShape &geoShape() const override;
    void setGeoShape(const QGeoShape &shape) override;

    QDeclarativeMapLineProperties *line();

Q_SIGNALS:
    void pathChanged();

protected Q_SLOTS:
    void updateAfterLinePropertiesChanged();
    void afterViewportChanged(const QGeoMapViewportChangeEvent &event) override;

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void setPathFromGeoList(const QList<QGeoCoordinate> &path);
    void updatePolish() override;

#ifdef QT_LOCATION_DEBUG
public:
#endif
    QGeoPath m_geopath;
    QDeclarativeMapLineProperties m_line;

    bool m_updatingGeometry = false;

    std::unique_ptr<QDeclarativePolylineMapItemPrivate> m_d;

    friend class QDeclarativePolylineMapItemPrivate;
    friend class QDeclarativePolylineMapItemPrivateCPU;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeMapLineProperties)
QML_DECLARE_TYPE(QDeclarativePolylineMapItem)

#endif /* QDECLARATIVEPOLYLINEMAPITEM_H_ */
