// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOMAP_H
#define QDECLARATIVEGEOMAP_H

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
#include <QtLocation/private/qdeclarativegeomapitemview_p.h>
#include <QtLocation/private/qdeclarativegeomapitemgroup_p.h>
#include <QtLocation/qgeoserviceprovider.h>
#include <QtLocation/private/qgeocameradata_p.h>
#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtQuick/QQuickItem>
#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtGui/QColor>
#include <QtPositioning/qgeorectangle.h>
#include <QtLocation/private/qgeomap_p.h>

Q_MOC_INCLUDE(<QtLocation/private/qdeclarativegeoserviceprovider_p.h>)

QT_BEGIN_NAMESPACE

class QQuickWindow;
class QDeclarativeGeoServiceProvider;
class QGeoMapType;
class QDeclarativeGeoMapCopyrightNotice;

class Q_LOCATION_EXPORT QDeclarativeGeoMap : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Map)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QDeclarativeGeoServiceProvider *plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(qreal minimumZoomLevel READ minimumZoomLevel WRITE setMinimumZoomLevel NOTIFY minimumZoomLevelChanged)
    Q_PROPERTY(qreal maximumZoomLevel READ maximumZoomLevel WRITE setMaximumZoomLevel NOTIFY maximumZoomLevelChanged)
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)

    Q_PROPERTY(qreal tilt READ tilt WRITE setTilt NOTIFY tiltChanged)
    Q_PROPERTY(qreal minimumTilt READ minimumTilt WRITE setMinimumTilt NOTIFY minimumTiltChanged)
    Q_PROPERTY(qreal maximumTilt READ maximumTilt WRITE setMaximumTilt NOTIFY maximumTiltChanged)

    Q_PROPERTY(qreal bearing READ bearing WRITE setBearing NOTIFY bearingChanged)

    Q_PROPERTY(qreal fieldOfView READ fieldOfView WRITE setFieldOfView NOTIFY fieldOfViewChanged)
    Q_PROPERTY(qreal minimumFieldOfView READ minimumFieldOfView WRITE setMinimumFieldOfView NOTIFY minimumFieldOfViewChanged)
    Q_PROPERTY(qreal maximumFieldOfView READ maximumFieldOfView WRITE setMaximumFieldOfView NOTIFY minimumFieldOfViewChanged)

    Q_PROPERTY(QGeoMapType activeMapType READ activeMapType WRITE setActiveMapType NOTIFY activeMapTypeChanged)
    Q_PROPERTY(QList<QGeoMapType> supportedMapTypes READ supportedMapTypes NOTIFY supportedMapTypesChanged)
    Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
    Q_PROPERTY(QList<QObject *> mapItems READ mapItems NOTIFY mapItemsChanged)
    Q_PROPERTY(QGeoServiceProvider::Error error READ error NOTIFY errorChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
    Q_PROPERTY(QGeoShape visibleRegion READ visibleRegion WRITE setVisibleRegion NOTIFY visibleRegionChanged)
    Q_PROPERTY(bool copyrightsVisible READ copyrightsVisible WRITE setCopyrightsVisible NOTIFY copyrightsVisibleChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(bool mapReady READ mapReady NOTIFY mapReadyChanged)
    Q_PROPERTY(QRectF visibleArea READ visibleArea WRITE setVisibleArea NOTIFY visibleAreaChanged  REVISION(5, 12))
    Q_INTERFACES(QQmlParserStatus)

public:

    explicit QDeclarativeGeoMap(QQuickItem *parent = nullptr);
    ~QDeclarativeGeoMap();

    void setPlugin(QDeclarativeGeoServiceProvider *plugin);
    QDeclarativeGeoServiceProvider *plugin() const;

    void setActiveMapType(const QGeoMapType &mapType);
    QGeoMapType activeMapType() const;

    void setMinimumZoomLevel(qreal minimumZoomLevel, bool userSet = true);
    qreal minimumZoomLevel() const;

    void setMaximumZoomLevel(qreal maximumZoomLevel, bool userSet = true);
    qreal maximumZoomLevel() const;

    void setZoomLevel(qreal zoomLevel);
    qreal zoomLevel() const;

    void setBearing(qreal bearing);
    qreal bearing() const;

    void setTilt(qreal tilt);
    qreal tilt() const;
    void setMinimumTilt(qreal minimumTilt, bool userSet = true);
    qreal minimumTilt() const;
    void setMaximumTilt(qreal maximumTilt, bool userSet = true);
    qreal maximumTilt() const;

    void setFieldOfView(qreal fieldOfView);
    qreal fieldOfView() const;
    void setMinimumFieldOfView(qreal minimumFieldOfView, bool userSet = true);
    qreal minimumFieldOfView() const;
    void setMaximumFieldOfView(qreal maximumFieldOfView, bool userSet = true);
    qreal maximumFieldOfView() const;

    void setCenter(const QGeoCoordinate &center);
    QGeoCoordinate center() const;

    void setVisibleRegion(const QGeoShape &shape);
    QGeoShape visibleRegion() const;

    void setCopyrightsVisible(bool visible);
    bool copyrightsVisible() const;

    void setColor(const QColor &color);
    QColor color() const;

    QRectF visibleArea() const;
    void setVisibleArea(const QRectF &visibleArea);

    bool mapReady() const;

    QList<QGeoMapType> supportedMapTypes();

    Q_INVOKABLE void setBearing(qreal bearing, const QGeoCoordinate &coordinate);
    Q_INVOKABLE void alignCoordinateToPoint(const QGeoCoordinate &coordinate, const QPointF &point);

    Q_INVOKABLE void removeMapItem(QDeclarativeGeoMapItemBase *item);
    Q_INVOKABLE void addMapItem(QDeclarativeGeoMapItemBase *item);

    Q_INVOKABLE void addMapItemGroup(QDeclarativeGeoMapItemGroup *itemGroup);
    Q_INVOKABLE void removeMapItemGroup(QDeclarativeGeoMapItemGroup *itemGroup);

    Q_INVOKABLE void removeMapItemView(QDeclarativeGeoMapItemView *itemView);
    Q_INVOKABLE void addMapItemView(QDeclarativeGeoMapItemView *itemView);

    Q_INVOKABLE void clearMapItems();
    QList<QObject *> mapItems();

    Q_INVOKABLE QGeoCoordinate toCoordinate(const QPointF &position, bool clipToViewPort = true) const;
    Q_INVOKABLE QPointF fromCoordinate(const QGeoCoordinate &coordinate, bool clipToViewPort = true) const;

//    QQuickGeoMapGestureArea *gesture();

    Q_INVOKABLE void fitViewportToMapItems(const QVariantList &items = {});
    Q_INVOKABLE void fitViewportToVisibleMapItems();
    Q_INVOKABLE void pan(int dx, int dy);
    Q_INVOKABLE void prefetchData(); // optional hint for prefetch
    Q_INVOKABLE void clearData();
    Q_REVISION(5, 13) Q_INVOKABLE void fitViewportToGeoShape(const QGeoShape &shape, QVariant margins);
    void fitViewportToGeoShape(const QGeoShape &shape, const QMargins &borders = QMargins(10, 10, 10, 10));

    QString errorString() const;
    QGeoServiceProvider::Error error() const;
    QGeoMap* map() const;

    // From QQuickItem
    void itemChange(ItemChange, const ItemChangeData &) override;

Q_SIGNALS:
    void pluginChanged(QDeclarativeGeoServiceProvider *plugin);
    void zoomLevelChanged(qreal zoomLevel);
    void centerChanged(const QGeoCoordinate &coordinate);
    void activeMapTypeChanged();
    void supportedMapTypesChanged();
    void minimumZoomLevelChanged(qreal minimumZoomLevel);
    void maximumZoomLevelChanged(qreal maximumZoomLevel);
    void mapItemsChanged();
    void errorChanged();
    void copyrightLinkActivated(const QString &link);
    void copyrightsVisibleChanged(bool visible);
    void colorChanged(const QColor &color);
    void bearingChanged(qreal bearing);
    void tiltChanged(qreal tilt);
    void fieldOfViewChanged(qreal fieldOfView);
    void minimumTiltChanged(qreal minimumTilt);
    void maximumTiltChanged(qreal maximumTilt);
    void minimumFieldOfViewChanged(qreal minimumFieldOfView);
    void maximumFieldOfViewChanged(qreal maximumFieldOfView);
    void copyrightsImageChanged(const QImage &copyrightsImage);
    void copyrightsChanged(const QString &copyrightsHtml);
    void mapReadyChanged(bool ready);
    void visibleAreaChanged();
    Q_REVISION(5, 14) void visibleRegionChanged();

protected:
    void componentComplete() override;
    QSGNode *updatePaintNode(QSGNode *, UpdatePaintNodeData *) override;
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    void setError(QGeoServiceProvider::Error error, const QString &errorString);
    void initialize();
    void setZoomLevel(qreal zoomLevel, bool overzoom);
    bool addMapChild(QObject *child);
    bool removeMapChild(QObject *child);
    bool isGroupNested(QDeclarativeGeoMapItemGroup *group) const;

    bool addMapItem_real(QDeclarativeGeoMapItemBase *item);
    bool removeMapItem_real(QDeclarativeGeoMapItemBase *item);
    bool addMapItemGroup_real(QDeclarativeGeoMapItemGroup *itemGroup);
    bool removeMapItemGroup_real(QDeclarativeGeoMapItemGroup *itemGroup);
    bool addMapItemView_real(QDeclarativeGeoMapItemView *itemView);
    bool removeMapItemView_real(QDeclarativeGeoMapItemView *itemView);
    void updateItemToWindowTransform();
    void onSGNodeChanged();

private Q_SLOTS:
    void mappingManagerInitialized();
    void pluginReady();
    void onSupportedMapTypesChanged();
    void onCameraCapabilitiesChanged(const QGeoCameraCapabilities &oldCameraCapabilities);
    void onAttachedCopyrightNoticeVisibilityChanged();
    void onCameraDataChanged(const QGeoCameraData &cameraData);

private:
    void setupMapView(QDeclarativeGeoMapItemView *view);
    void populateMap();
    void fitViewportToMapItemsRefine(const QList<QPointer<QDeclarativeGeoMapItemBase> > &mapItems, bool refine, bool onlyVisible);
    void attachCopyrightNotice(bool initialVisibility);
    void detachCopyrightNotice(bool currentVisibility);
    QMargins mapMargins() const;

private:
    QQuickWindow *m_window = nullptr;
    QDeclarativeGeoServiceProvider *m_plugin = nullptr;
    QGeoMappingManager *m_mappingManager = nullptr;
    QGeoMapType m_activeMapType;
    QList<QGeoMapType> m_supportedMapTypes;
    QList<QDeclarativeGeoMapItemView *> m_mapViews;
    QPointer<QGeoMap> m_map;
    QPointer<QDeclarativeGeoMapCopyrightNotice> m_copyrights;
    QList<QPointer<QDeclarativeGeoMapItemBase> > m_mapItems;
    QList<QPointer<QDeclarativeGeoMapItemGroup> > m_mapItemGroups;
    QString m_errorString;
    QGeoServiceProvider::Error m_error = QGeoServiceProvider::NoError;
    QGeoRectangle m_visibleRegion;
    QColor m_color = QColor::fromRgbF(0.9f, 0.9f, 0.9f);
    QGeoCameraData m_cameraData;
    bool m_componentCompleted = false;
    bool m_pendingFitViewport = false;
    bool m_copyrightsVisible = true;
    double m_maximumViewportLatitude = 0.0;
    double m_minimumViewportLatitude = 0.0;
    bool m_initialized = false;
    bool m_sgNodeHasChanged = false;
    QGeoCameraCapabilities m_cameraCapabilities;
    qreal m_minimumZoomLevel = Q_QNAN;
    qreal m_maximumZoomLevel = Q_QNAN;
    qreal m_userMinimumZoomLevel = 0;
    qreal m_userMaximumZoomLevel = Q_INFINITY;

    qreal m_minimumTilt = Q_QNAN;
    qreal m_maximumTilt = Q_QNAN;
    qreal m_userMinimumTilt = 0;
    qreal m_userMaximumTilt = Q_INFINITY;

    qreal m_minimumFieldOfView = Q_QNAN;
    qreal m_maximumFieldOfView = Q_QNAN;
    qreal m_userMinimumFieldOfView = 0;
    qreal m_userMaximumFieldOfView = Q_INFINITY;

    int m_copyNoticesVisible = 0;
    qreal m_maxChildZ = 0;
    QRectF m_visibleArea;


    friend class QDeclarativeGeoMapItem;
    friend class QDeclarativeGeoMapItemView;
    friend class QDeclarativeGeoMapCopyrightNotice;
    Q_DISABLE_COPY(QDeclarativeGeoMap)
};


QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeGeoMap)

#endif
