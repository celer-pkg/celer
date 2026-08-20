// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QGEOMAP_P_H
#define QGEOMAP_P_H

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
#include <QtLocation/private/qgeomaptype_p.h>
#include <QtCore/QObject>
#include <QTransform>

QT_BEGIN_NAMESPACE

class QGeoMappingManagerEngine;
class QGeoMapPrivate;
class QGeoMapController;
class QGeoCameraCapabilities;
class QGeoCameraData;
class QGeoCoordinate;
class QGeoProjection;
class QGeoShape;
class QGeoRectangle;
class QSGNode;
class QQuickWindow;
class QDeclarativeGeoMapItemBase;
class QDeclarativeGeoMap;

class Q_LOCATION_EXPORT QGeoMap : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QGeoMap)

public:
    enum ItemType {
        NoItem = 0x0000,
        MapRectangle = 0x0001,
        MapCircle = 0x0002,
        MapPolyline = 0x0004,
        MapPolygon = 0x0008,
        MapQuickItem = 0x0010,
        CustomMapItem = 0x8000
    };

    Q_DECLARE_FLAGS(ItemTypes, ItemType)

    enum Capability {
        SupportsNothing = 0x0000,
        SupportsVisibleRegion = 0x0001,
        SupportsSetBearing = 0x0002,
        SupportsAnchoringCoordinate = 0x0004,
        SupportsFittingViewportToGeoRectangle = 0x0008,
        SupportsVisibleArea = 0x0010,
    };

    Q_ENUM(Capability)
    Q_DECLARE_FLAGS(Capabilities, Capability)
    Q_FLAG(Capabilities)

    virtual ~QGeoMap();

    // Sets the display size
    void setViewportSize(const QSize& viewportSize);
    QSize viewportSize() const;
    int viewportWidth() const;
    int viewportHeight() const;


    const QGeoCameraData &cameraData() const;
    QGeoCameraCapabilities cameraCapabilities() const;
    virtual Capabilities capabilities() const;

    void setActiveMapType(const QGeoMapType &mapType);
    QGeoMapType activeMapType() const;

    // returns the minimum zoom at the current viewport size
    double minimumZoom() const;
    double maximumCenterLatitudeAtZoom(const QGeoCameraData &cameraData) const;
    double minimumCenterLatitudeAtZoom(const QGeoCameraData &cameraData) const;

    // returns the size of the underlying map, at the current zoom level. Unrelated to width()/height()/size().
    double mapWidth() const;
    double mapHeight() const;

    const QGeoProjection &geoProjection() const;

    virtual void prefetchData();
    virtual void clearData();

    ItemTypes supportedMapItemTypes() const;

    void addMapItem(QDeclarativeGeoMapItemBase *item);
    void removeMapItem(QDeclarativeGeoMapItemBase *item);
    void clearMapItems();

    virtual QString copyrightsStyleSheet() const;
    virtual void setAcceptedGestures(bool pan, bool flick, bool pinch, bool rotate, bool tilt);
    virtual bool handleEvent(QEvent *event);

    virtual bool setBearing(qreal bearing, const QGeoCoordinate &coordinate);
    virtual QGeoShape visibleRegion() const;
    virtual bool anchorCoordinateToPoint(const QGeoCoordinate &coordinate, const QPointF &anchorPoint);
    virtual bool fitViewportToGeoRectangle(const QGeoRectangle &rectangle, const QMargins &borders);

    virtual void setCopyrightVisible(bool visible);
    virtual void setItemToWindowTransform(const QTransform &itemToWindowTransform);

    void setVisibleArea(const QRectF &visibleArea);
    QRectF visibleArea() const;

protected:
    QGeoMap(QGeoMapPrivate &dd, QObject *parent = nullptr);
    void setCameraData(const QGeoCameraData &cameraData);
    void setCameraCapabilities(const QGeoCameraCapabilities &cameraCapabilities);
    virtual QSGNode *updateSceneGraph(QSGNode *node, QQuickWindow *window) = 0;

Q_SIGNALS:
    void cameraDataChanged(const QGeoCameraData &cameraData);
    void sgNodeChanged();
    void activeMapTypeChanged();
    void cameraCapabilitiesChanged(const QGeoCameraCapabilities &oldCameraCapabilities);
    void copyrightsImageChanged(const QImage &copyrightsImage);
    void copyrightsChanged(const QString &copyrightsHtml);
    void copyrightsStyleSheetChanged(const QString &styleSheet);
    void visibleAreaChanged();

private:
    Q_DISABLE_COPY(QGeoMap)
    friend class QDeclarativeGeoMap; //updateSceneGraph
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QGeoMap::ItemTypes)

QT_END_NAMESPACE

#endif // QGEOMAP_P_H
