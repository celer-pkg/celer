// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOMAPITEMBASE_H
#define QDECLARATIVEGEOMAPITEMBASE_H

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
#include <QtPositioning/QGeoShape>

#include <QtLocation/qlocation.h>
#include <QtLocation/private/qdeclarativegeomap_p.h>
#include <QtLocation/private/qlocationglobal_p.h>
#include <QtLocation/private/qgeomap_p.h>
#include <QtLocation/private/qdeclarativegeomapitemtransitionmanager_p.h>
#include <QScopedPointer>
#include <QtQuickShapes/private/qquickshape_p.h>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

struct Q_LOCATION_EXPORT QGeoMapViewportChangeEvent
{
    QGeoCameraData cameraData;
    QSizeF mapSize;

    bool zoomLevelChanged = false;
    bool centerChanged = false;
    bool mapSizeChanged = false;
    bool tiltChanged = false;
    bool bearingChanged = false;
    bool rollChanged = false;
};

class Q_LOCATION_EXPORT QDeclarativeGeoMapItemBase : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GeoMapItemBase)
    QML_ADDED_IN_VERSION(5, 0)
    QML_UNCREATABLE("GeoMapItemBase is not intended instantiable by developer.")

    Q_PROPERTY(QGeoShape geoShape READ geoShape WRITE setGeoShape STORED false )
    Q_PROPERTY(bool autoFadeIn READ autoFadeIn WRITE setAutoFadeIn REVISION(5, 14))
    Q_PROPERTY(QLocation::ReferenceSurface referenceSurface READ referenceSurface WRITE setReferenceSurface NOTIFY referenceSurfaceChanged REVISION(6, 6))
    Q_PROPERTY(int lodThreshold READ lodThreshold WRITE setLodThreshold NOTIFY lodThresholdChanged REVISION(5, 15))

public:
    explicit QDeclarativeGeoMapItemBase(QQuickItem *parent = nullptr);
    virtual ~QDeclarativeGeoMapItemBase();


    virtual void setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map);
    virtual void setPositionOnMap(const QGeoCoordinate &coordinate, const QPointF &offset);

    QDeclarativeGeoMap *quickMap() const { return quickMap_; }
    QGeoMap *map() const { return map_; }
    virtual const QGeoShape &geoShape() const = 0;
    virtual void setGeoShape(const QGeoShape &shape) = 0;

    bool autoFadeIn() const;
    void setAutoFadeIn(bool fadeIn);

    QLocation::ReferenceSurface referenceSurface() const;
    void setReferenceSurface(QLocation::ReferenceSurface referenceSurface);

    int lodThreshold() const;
    void setLodThreshold(int lt);
    unsigned int zoomForLOD(int zoom) const;

    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    virtual QSGNode *updateMapItemPaintNode(QSGNode *, UpdatePaintNodeData *);

    inline QGeoMap::ItemType itemType() const { return m_itemType; }
    qreal mapItemOpacity() const;

    void setParentGroup(QDeclarativeGeoMapItemGroup &parentGroup);

    template <typename T = QObject>
    QList<T*> quickChildren() const
    {
        QList<T*> res;
        QObjectList kids = children();
        const QList<QQuickItem *> quickKids = childItems();
        for (const auto &quickKid : quickKids)
            kids.append(quickKid);
        for (auto *kid : std::as_const(kids)) {
            if (auto *val = qobject_cast<T*>(kid))
                res.push_back(val);
        }
        return res;
    }

    void setShapeTriangulationScale(QQuickShape *shape, qreal maxCoord) const;

Q_SIGNALS:
    void mapItemOpacityChanged();
    Q_REVISION(5, 12) void addTransitionFinished();
    Q_REVISION(5, 12) void removeTransitionFinished();
    void referenceSurfaceChanged();
    void lodThresholdChanged();

protected Q_SLOTS:
    virtual void afterChildrenChanged();
    virtual void afterViewportChanged(const QGeoMapViewportChangeEvent &event) = 0;
    void polishAndUpdate();

protected:
    float zoomLevelOpacity() const;
    bool isPolishScheduled() const;

    QGeoMap::ItemType m_itemType = QGeoMap::NoItem;

private Q_SLOTS:
    void baseCameraDataChanged(const QGeoCameraData &camera);
    void visibleAreaChanged();

private:
    QPointer<QGeoMap> map_;
    QDeclarativeGeoMap *quickMap_ = nullptr;

    QSizeF lastMapSize_;
    QGeoCameraData lastCameraData_;

    QDeclarativeGeoMapItemGroup *parentGroup_ = nullptr;

    std::unique_ptr<QDeclarativeGeoMapItemTransitionManager> m_transitionManager;
    bool m_autoFadeIn = true;
    QLocation::ReferenceSurface m_referenceSurface = QLocation::ReferenceSurface::Map;
    int m_lodThreshold = 0;

    friend class QDeclarativeGeoMap;
    friend class QDeclarativeGeoMapItemView;
    friend class QDeclarativeGeoMapItemTransitionManager;
};

class QDeclarativeGeoMapPainterPath : public QQuickCurve
{
    Q_OBJECT
public:
    QDeclarativeGeoMapPainterPath(QObject *parent = nullptr) : QQuickCurve(parent) {}
    QPainterPath path() const
    {
        return m_path;
    }
    void setPath(const QPainterPath &path)
    {
        m_path = path;
        emit changed();
    }
    void addToPath(QPainterPath &path, const QQuickPathData &) override
    {
        path.addPath(m_path);
    }
private:
    QPainterPath m_path;
};

QT_END_NAMESPACE

#endif
