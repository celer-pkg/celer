// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativegeomap_p.h"
#include "qdeclarativegeomapquickitem_p.h"
#include "qdeclarativegeomapcopyrightsnotice_p.h"
#include "qdeclarativegeoserviceprovider_p.h"
#include "qgeomappingmanager_p.h"
#include "qgeocameracapabilities_p.h"
#include "qgeomap_p.h"
#include "qgeoprojection_p.h"
#include <QtPositioning/QGeoCircle>
#include <QtPositioning/QGeoRectangle>
#include <QtPositioning/QGeoPath>
#include <QtPositioning/QGeoPolygon>
#include <QtQuick/QQuickWindow>
#include <QtQuick/QSGRectangleNode>
#include <QtQml/qqmlinfo.h>
#include <QtQuick/private/qquickitem_p.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.141592653589793238463
#endif


QT_BEGIN_NAMESPACE

static qreal sanitizeBearing(qreal bearing)
{
    bearing = std::fmod(bearing, qreal(360.0));
    if (bearing < 0.0)
        bearing += 360.0;

    return bearing;
}

/*!
    \qmltype Map
    \nativetype QDeclarativeGeoMap
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.0

    \brief The Map type displays a map.

    The Map type is used to display a map or image of the Earth, with
    the capability to also display interactive objects tied to the map's
    surface.

    There are a variety of different ways to visualize the Earth's surface
    in a 2-dimensional manner, but all of them involve some kind of projection:
    a mathematical relationship between the 3D coordinates (latitude, longitude
    and altitude) and 2D coordinates (X and Y in pixels) on the screen.

    Different sources of map data can use different projections, and from the
    point of view of the Map type, we treat these as one replaceable unit:
    the Map plugin. A Map plugin consists of a data source, as well as all other
    details needed to display its data on-screen.

    The current Map plugin in use is contained in the \l plugin property of
    the Map item. In order to display any image in a Map item, you will need
    to set this property. See the \l Plugin type for a description of how
    to retrieve an appropriate plugin for use.

    The geographic region displayed in the Map item is referred to as its
    viewport, and this is defined by the properties \l center, and
    \l zoomLevel. The \l center property contains a \l {coordinate}
    specifying the center of the viewport, while \l zoomLevel controls the scale of the
    map. See each of these properties for further details about their values.

    When the map is displayed, each possible geographic coordinate that is
    visible will map to some pixel X and Y coordinate on the screen. To perform
    conversions between these two, Map provides the \l toCoordinate and
    \l fromCoordinate functions, which are of general utility.

    \section2 Map Objects

    Map related objects can be declared within the body of a Map object in Qt Quick and will
    automatically appear on the Map. To add an object programmatically, first be
    sure it is created with the Map as its parent (for example in an argument to
    Component::createObject).
    Then call the \l addMapItem method on the Map, if the type of this object is one of
    \l MapCircle, \l MapRectangle, \l MapPolyline, \l MapPolygon, \l MapRoute or \l MapQuickItem.
    A corresponding \l removeMapItem method also exists to do the opposite and
    remove any of the above types of map objects from the Map.

    Moving Map objects around, resizing them or changing their shape normally
    does not involve any special interaction with Map itself -- changing these
    properties in a map object will automatically update the display.

    \section2 Performance

    Maps are rendered using OpenGL (ES) and the Qt Scene Graph stack, and as
    a result perform quite well where GL accelerated hardware is available.

    For "online" Maps, network bandwidth and latency can be major
    contributors to the user's perception of performance. Extensive caching is
    performed to mitigate this, but such mitigation is not always perfect.

    In general, large and complex Map items such as polygons and polylines with
    large numbers of vertices can have an adverse effect on UI performance.

    \section2 Example Usage

    The following snippet shows a simple Map and the necessary Plugin type
    to use it. The map is centered over Oslo, Norway, with zoom level 14.

    \quotefromfile minimal_map/main.qml
    \skipto import
    \printuntil Window {
    \dots
    \skipto Plugin
    \printuntil

    \image minimal_map.png
*/

/*!
    \qmlsignal QtLocation::Map::copyrightLinkActivated(string link)

    This signal is emitted when the user clicks on a \a link in the copyright notice. The
    application should open the link in a browser or display its contents to the user.
*/

QDeclarativeGeoMap::QDeclarativeGeoMap(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlags(QQuickItem::ItemHasContents | QQuickItem::ItemClipsChildrenToShape);

    m_activeMapType = QGeoMapType(QGeoMapType::NoMap,
                                  tr("No Map"),
                                  tr("No Map"),
                                  false, false,
                                  0,
                                  QByteArrayLiteral(""),
                                  QGeoCameraCapabilities());
    m_cameraData.setCenter(QGeoCoordinate(51.5073,-0.1277)); //London city center
    m_cameraData.setZoomLevel(8.0);

    m_cameraCapabilities.setTileSize(256);
    m_cameraCapabilities.setSupportsBearing(true);
    m_cameraCapabilities.setSupportsTilting(true);
    m_cameraCapabilities.setMinimumZoomLevel(0);
    m_cameraCapabilities.setMaximumZoomLevel(30);
    m_cameraCapabilities.setMinimumTilt(0);
    m_cameraCapabilities.setMaximumTilt(89.5);
    m_cameraCapabilities.setMinimumFieldOfView(1);
    m_cameraCapabilities.setMaximumFieldOfView(179);

    m_minimumZoomLevel = m_cameraCapabilities.minimumZoomLevel();
    m_maximumZoomLevel = m_cameraCapabilities.maximumZoomLevel();
    m_minimumTilt = m_cameraCapabilities.minimumTilt();
    m_maximumTilt = m_cameraCapabilities.maximumTilt();
    m_minimumFieldOfView = m_cameraCapabilities.minimumFieldOfView();
    m_maximumFieldOfView = m_cameraCapabilities.maximumFieldOfView();
}

QDeclarativeGeoMap::~QDeclarativeGeoMap()
{
    // Removing map items from m_map
    if (m_map) {
        m_map->clearMapItems();
    }

    // Remove the items from the map, making them deletable.
    // Go in the same order as in removeMapChild: views, groups, then items
    if (!m_mapViews.isEmpty()) {
        const auto mapViews = m_mapViews;
        for (QDeclarativeGeoMapItemView *v : mapViews) { // so that removeMapItemView_real can safely modify m_mapViews;
            if (!v)
                continue;

            QQuickItem *parent = v->parentItem();
            QDeclarativeGeoMapItemGroup *group = qobject_cast<QDeclarativeGeoMapItemGroup *>(parent);
            if (group)
                continue; // Ignore non-top-level MIVs. They will be recursively processed.
                          // Identify them as being parented by a MapItemGroup.

            removeMapItemView_real(v);
        }
    }

    if (!m_mapItemGroups.isEmpty()) {
        const auto mapGroups = m_mapItemGroups;
        for (QDeclarativeGeoMapItemGroup *g : mapGroups) {
            if (!g)
                continue;

            QQuickItem *parent =g->parentItem();
            QDeclarativeGeoMapItemGroup *group = qobject_cast<QDeclarativeGeoMapItemGroup *>(parent);
            if (group)
                continue; // Ignore non-top-level Groups. They will be recursively processed.
                          // Identify them as being parented by a MapItemGroup.

            removeMapItemGroup_real(g);
        }
    }

    // remove any remaining map items associations
    const auto mapItems = m_mapItems;
    for (auto mi: mapItems)
        removeMapItem_real(mi.data());

    if (m_copyrights.data())
        delete m_copyrights.data();
    m_copyrights.clear();

    delete m_map; // map objects get reset here
}

void QDeclarativeGeoMap::onSupportedMapTypesChanged()
{
    m_supportedMapTypes = m_mappingManager->supportedMapTypes();
    if (m_supportedMapTypes.isEmpty()) {
        m_map->setActiveMapType(QGeoMapType()); // no supported map types: setting an invalid one
    } else if (!m_supportedMapTypes.contains(m_map->activeMapType())) {
        QGeoMapType type = m_supportedMapTypes.at(0);
        m_activeMapType = type;
        m_map->setActiveMapType(type);
    }

    emit supportedMapTypesChanged();
}

void QDeclarativeGeoMap::setError(QGeoServiceProvider::Error error, const QString &errorString)
{
    if (m_error == error && m_errorString == errorString)
        return;
    m_error = error;
    m_errorString = errorString;
    emit errorChanged();
}

/*!
    \internal
    Called when the mapping manager is initialized AND the declarative element has a valid size > 0
*/
void QDeclarativeGeoMap::initialize()
{
    // try to keep change signals in the end
    bool visibleAreaHasChanged = false;

    QGeoCoordinate center = m_cameraData.center();

    setMinimumZoomLevel(m_map->minimumZoom(), false);

    double bearing = m_cameraData.bearing();
    double tilt = m_cameraData.tilt();
    double fov = m_cameraData.fieldOfView(); // Must be 45.0
    QGeoCameraData cameraData = m_cameraData;

    if (!m_cameraCapabilities.supportsBearing() && bearing != 0.0)
        cameraData.setBearing(0);

    if (!m_cameraCapabilities.supportsTilting() && tilt != 0.0)
        cameraData.setTilt(0);

    m_map->setVisibleArea(m_visibleArea);
    if (m_map->visibleArea() != m_visibleArea)
        visibleAreaHasChanged = true;

    cameraData.setFieldOfView(qBound(m_cameraCapabilities.minimumFieldOfView(),
                                       fov,
                                       m_cameraCapabilities.maximumFieldOfView()));

    // set latitude boundary check
    m_maximumViewportLatitude = m_map->maximumCenterLatitudeAtZoom(cameraData);
    m_minimumViewportLatitude = m_map->minimumCenterLatitudeAtZoom(cameraData);

    center.setLatitude(qBound(m_minimumViewportLatitude, center.latitude(), m_maximumViewportLatitude));
    cameraData.setCenter(center);

    connect(m_map.data(), &QGeoMap::cameraDataChanged,
            this,  &QDeclarativeGeoMap::onCameraDataChanged);
    m_map->setCameraData(cameraData); // This normally triggers property changed signals.
                                      // BUT not in this case, since m_cameraData is already == cameraData.
                                      // So, emit visibleRegionChanged() separately, as
                                      // the effective visible region becomes available only now.

    m_initialized = true;

    if (visibleAreaHasChanged)
        emit visibleAreaChanged();
    connect(m_map.data(), &QGeoMap::visibleAreaChanged, this, &QDeclarativeGeoMap::visibleAreaChanged);

    emit mapReadyChanged(true);
    emit visibleRegionChanged();

    if (m_copyrights) // To not update during initialize()
         update();
}

/*!
    \internal
*/
void QDeclarativeGeoMap::pluginReady()
{
    QGeoServiceProvider *provider = m_plugin->sharedGeoServiceProvider();
    m_mappingManager = provider->mappingManager();

    if (provider->mappingError() != QGeoServiceProvider::NoError) {
        setError(provider->mappingError(), provider->mappingErrorString());
        return;
    }

    if (!m_mappingManager) {
        //TODO Should really be EngineNotSetError (see QML GeoCodeModel)
        setError(QGeoServiceProvider::NotSupportedError, tr("Plugin does not support mapping."));
        return;
    }

    if (!m_mappingManager->isInitialized()) {
        connect(m_mappingManager, &QGeoMappingManager::initialized,
                this, &QDeclarativeGeoMap::mappingManagerInitialized);
    } else {
        mappingManagerInitialized();
    }

    // make sure this is only called once
    disconnect(m_plugin, &QDeclarativeGeoServiceProvider::attached,
               this, &QDeclarativeGeoMap::pluginReady);
}

/*!
    \internal
*/
void QDeclarativeGeoMap::componentComplete()
{
    m_componentCompleted = true;
    populateMap();
    QQuickItem::componentComplete();
}

/*!
    \internal

    This may happen before mappingManagerInitialized()
*/
void QDeclarativeGeoMap::populateMap()
{
    QSet<QObject *> kids(children().cbegin(), children().cend());
    const QList<QQuickItem *> quickKids = childItems();
    for (QQuickItem *ite: quickKids)
        kids.insert(ite);

    for (QObject *k : std::as_const(kids)) {
        addMapChild(k);
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMap::setupMapView(QDeclarativeGeoMapItemView *view)
{
    view->setMap(this);
}

/*!
 * \internal
 */
QSGNode *QDeclarativeGeoMap::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    if (!m_map) {
        delete oldNode;
        return nullptr;
    }

    QSGRectangleNode *root = static_cast<QSGRectangleNode *>(oldNode);
    if (!root)
        root = window()->createRectangleNode();

    root->setRect(boundingRect());
    root->setColor(m_color);

    QSGNode *content = root->childCount() ? root->firstChild() : 0;
    content = m_map->updateSceneGraph(content, window());
    if (content && root->childCount() == 0)
        root->appendChildNode(content);

    return root;
}

/*!
    \qmlproperty Plugin QtLocation::Map::plugin

    This property holds the plugin which provides the mapping functionality.

    This is a write-once property. Once the map has a plugin associated with
    it, any attempted modifications of the plugin will be ignored.
*/

void QDeclarativeGeoMap::setPlugin(QDeclarativeGeoServiceProvider *plugin)
{
    if (m_plugin) {
        qmlWarning(this) << QStringLiteral("Plugin is a write-once property, and cannot be set again.");
        return;
    }
    m_plugin = plugin;
    emit pluginChanged(m_plugin);

    if (m_plugin->isAttached()) {
        pluginReady();
    } else {
        connect(m_plugin, &QDeclarativeGeoServiceProvider::attached,
                this, &QDeclarativeGeoMap::pluginReady);
    }
}

/*!
    \internal
*/
void QDeclarativeGeoMap::onCameraCapabilitiesChanged(const QGeoCameraCapabilities &oldCameraCapabilities)
{
    if (m_map->cameraCapabilities() == oldCameraCapabilities)
        return;

    m_cameraCapabilities = m_map->cameraCapabilities();

    setMaximumZoomLevel(m_cameraCapabilities.maximumZoomLevel(), false);
    setMinimumZoomLevel(m_cameraCapabilities.minimumZoomLevel(), false);
    setMinimumTilt(m_cameraCapabilities.minimumTilt(), false);
    setMaximumTilt(m_cameraCapabilities.maximumTilt(), false);
    setMinimumFieldOfView(m_cameraCapabilities.minimumFieldOfView(), false);
    setMaximumFieldOfView(m_cameraCapabilities.maximumFieldOfView(), false);
}

/*!
    \internal
    this function will only be ever called once
*/
void QDeclarativeGeoMap::mappingManagerInitialized()
{
    m_map = m_mappingManager->createMap(this);

    if (!m_map)
        return;

    // Any map items that were added before the plugin was ready
    // need to have setMap called again
    for (const QPointer<QDeclarativeGeoMapItemBase> &item : std::as_const(m_mapItems)) {
        if (item) {
            item->setMap(this, m_map);
            m_map->addMapItem(item.data()); // m_map filters out what is not supported.
        }
    }

    /* COPY NOTICE SETUP */
    m_copyrights = new QDeclarativeGeoMapCopyrightNotice(this);
    m_copyrights->setCopyrightsZ(m_maxChildZ + 1);
    m_copyrights->setCopyrightsVisible(m_copyrightsVisible);
    m_copyrights->setMapSource(this);

    m_supportedMapTypes = m_mappingManager->supportedMapTypes();

    if (m_activeMapType != QGeoMapType() && m_plugin->name().toLatin1() == m_activeMapType.pluginName()) {
        m_map->setActiveMapType(m_activeMapType);
    } else {
        if (!m_supportedMapTypes.isEmpty()) {
            m_activeMapType = m_supportedMapTypes.at(0);
            m_map->setActiveMapType(m_activeMapType);
        } else {
            m_activeMapType = QGeoMapType(QGeoMapType::NoMap,
                                          tr("No Map"),
                                          tr("No Map"),
                                          false,
                                          false,
                                          0,
                                          QByteArrayLiteral(""),
                                          QGeoCameraCapabilities());
        }
    }

    // Update camera capabilities
    onCameraCapabilitiesChanged(m_cameraCapabilities);

    // Map tiles are built in this call. m_map->minimumZoom() becomes operational
    // after this has been called at least once, after creation.
    // However, getting into the following block may fire a copyrightsChanged that would get lost,
    // as the connections are set up after.
    QString copyrightString;
    QImage copyrightImage;
    if (!m_initialized && width() > 0 && height() > 0) {
        QMetaObject::Connection copyrightStringCatcherConnection =
                connect(m_map.data(), &QGeoMap::copyrightsChanged, this,
                        [&copyrightString](const QString &copy){ copyrightString = copy; });
        QMetaObject::Connection copyrightImageCatcherConnection =
                connect(m_map.data(), &QGeoMap::copyrightsImageChanged, this,
                        [&copyrightImage](const QImage &copy){ copyrightImage = copy; });
        m_map->setViewportSize(QSize(width(), height()));
        initialize(); // This emits the caught signals above
        QObject::disconnect(copyrightStringCatcherConnection);
        QObject::disconnect(copyrightImageCatcherConnection);
    }


    /* COPYRIGHT SIGNALS REWIRING */
    connect(m_map.data(), &QGeoMap::copyrightsImageChanged,
            this, &QDeclarativeGeoMap::copyrightsImageChanged);
    connect(m_map.data(), &QGeoMap::copyrightsChanged,
            this, &QDeclarativeGeoMap::copyrightsChanged);
    if (!copyrightString.isEmpty())
        emit m_map->copyrightsChanged(copyrightString);
    else if (!copyrightImage.isNull())
        emit m_map->copyrightsImageChanged(copyrightImage);

    m_window = window();
    if (m_window) {
        connect(m_window, &QQuickWindow::beforeSynchronizing,
                this, &QDeclarativeGeoMap::updateItemToWindowTransform, Qt::DirectConnection);
    }
    connect(m_map.data(), &QGeoMap::sgNodeChanged, this, &QDeclarativeGeoMap::onSGNodeChanged);
    connect(m_map.data(), &QGeoMap::cameraCapabilitiesChanged,
            this, &QDeclarativeGeoMap::onCameraCapabilitiesChanged);

    // This prefetches a buffer around the map
    m_map->prefetchData();

    connect(m_mappingManager, &QGeoMappingManager::supportedMapTypesChanged,
            this, &QDeclarativeGeoMap::onSupportedMapTypesChanged);
    emit minimumZoomLevelChanged(minimumZoomLevel());
    emit maximumZoomLevelChanged(maximumZoomLevel());
    emit supportedMapTypesChanged();
    emit activeMapTypeChanged();

    // Any map item groups that were added before the plugin was ready
    // DO NOT need to have setMap called again on their children map items
    // because they have been added to m_mapItems, which is processed right above.

    if (m_initialized)
        update();
}

/*!
    \internal
*/
QDeclarativeGeoServiceProvider *QDeclarativeGeoMap::plugin() const
{
    return m_plugin;
}

/*!
    \internal
    Sets the gesture areas minimum zoom level. If the camera capabilities
    has been set this method honors the boundaries set by it.
    The minimum zoom level will also have a lower bound dependent on the size
    of the canvas, effectively preventing to display out of bounds areas.
*/
void QDeclarativeGeoMap::setMinimumZoomLevel(qreal minimumZoomLevel, bool userSet)
{
    if (minimumZoomLevel >= 0) {
        qreal oldMinimumZoomLevel = this->minimumZoomLevel();

        if (userSet)
            m_userMinimumZoomLevel = minimumZoomLevel;
        else
            m_minimumZoomLevel = minimumZoomLevel;

        if (zoomLevel() < minimumZoomLevel)
            setZoomLevel(minimumZoomLevel);

        if (oldMinimumZoomLevel != this->minimumZoomLevel())
            emit minimumZoomLevelChanged(this->minimumZoomLevel());
    }
}

/*!
    \qmlproperty real QtLocation::Map::minimumZoomLevel

    This property holds the minimum valid zoom level for the map.

    The minimum zoom level defined by the \l plugin used is a lower bound for
    this property. However, the returned value is also canvas-size-dependent, and
    can be higher than the user-specified value, or than the minimum zoom level
    defined by the plugin used, to prevent the map from being smaller than the
    viewport in either dimension.

    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 0.
*/

qreal QDeclarativeGeoMap::minimumZoomLevel() const
{
    return qMax(qMin(m_maximumZoomLevel, m_userMinimumZoomLevel), m_minimumZoomLevel);
}

/*!
    \internal
    Sets the gesture areas maximum zoom level. If the camera capabilities
    has been set this method honors the boundaries set by it.
*/
void QDeclarativeGeoMap::setMaximumZoomLevel(qreal maximumZoomLevel, bool userSet)
{
    if (maximumZoomLevel >= 0) {
        qreal oldMaximumZoomLevel = this->maximumZoomLevel();

        if (userSet)
            m_userMaximumZoomLevel = maximumZoomLevel;
        else
            m_maximumZoomLevel = maximumZoomLevel;

        if (zoomLevel() > maximumZoomLevel)
            setZoomLevel(maximumZoomLevel);

        if (oldMaximumZoomLevel != this->maximumZoomLevel())
            emit maximumZoomLevelChanged(this->maximumZoomLevel());
    }
}

/*!
    \qmlproperty real QtLocation::Map::maximumZoomLevel

    This property holds the maximum valid zoom level for the map.

    The maximum zoom level is defined by the \l plugin used.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 30.
*/

qreal QDeclarativeGeoMap::maximumZoomLevel() const
{
    return qMin(qMax(m_minimumZoomLevel, m_userMaximumZoomLevel), m_maximumZoomLevel);
}

/*!
    \qmlproperty real QtLocation::Map::zoomLevel

    This property holds the zoom level for the map.

    Larger values for the zoom level provide more detail. Zoom levels
    are always non-negative. The default value is 8.0. Depending on the plugin in use,
    values outside the [minimumZoomLevel, maximumZoomLevel] range, which represent the range for which
    tiles are available, may be accepted, or clamped.
*/
void QDeclarativeGeoMap::setZoomLevel(qreal zoomLevel)
{
    return setZoomLevel(zoomLevel, m_cameraCapabilities.overzoomEnabled());
}

/*!
    \internal

    Sets the zoom level.
    Larger values for the zoom level provide more detail. Zoom levels
    are always non-negative. The default value is 8.0. Values outside the
    [minimumZoomLevel, maximumZoomLevel] range, which represent the range for which
    tiles are available, can be accepted or clamped by setting the overzoom argument
    to true or false respectively.
*/
void QDeclarativeGeoMap::setZoomLevel(qreal zoomLevel, bool overzoom)
{
    if (zoomLevel < 0)
        return;

    if (m_initialized) {
        QGeoCameraData cameraData = m_map->cameraData();
        if (cameraData.zoomLevel() == zoomLevel)
            return;

        cameraData.setZoomLevel(qBound<qreal>(overzoom ? m_map->minimumZoom() : m_cameraCapabilities.minimumZoomLevel(),
                                                zoomLevel,
                                                overzoom ? 30 : maximumZoomLevel()));
        m_maximumViewportLatitude = m_map->maximumCenterLatitudeAtZoom(cameraData);
        m_minimumViewportLatitude = m_map->minimumCenterLatitudeAtZoom(cameraData);
        QGeoCoordinate coord = cameraData.center();
        coord.setLatitude(qBound(m_minimumViewportLatitude, coord.latitude(), m_maximumViewportLatitude));
        cameraData.setCenter(coord);
        m_map->setCameraData(cameraData);
    } else {
        const bool zlHasChanged = zoomLevel != m_cameraData.zoomLevel();
        m_cameraData.setZoomLevel(zoomLevel);
        if (zlHasChanged) {
            emit zoomLevelChanged(zoomLevel);
            // do not emit visibleRegionChanged() here, because, if the map isn't initialized,
            // the getter won't return anything updated
        }
    }
}

bool QDeclarativeGeoMap::addMapChild(QObject *child)
{
    // dispatch items appropriately
    QDeclarativeGeoMapItemView *mapView = qobject_cast<QDeclarativeGeoMapItemView *>(child);
    if (mapView)
        return addMapItemView_real(mapView);

    QDeclarativeGeoMapItemGroup *itemGroup = qobject_cast<QDeclarativeGeoMapItemGroup *>(child);
    if (itemGroup) // addMapItemView calls addMapItemGroup
        return addMapItemGroup_real(itemGroup);

    QDeclarativeGeoMapItemBase *mapItem = qobject_cast<QDeclarativeGeoMapItemBase *>(child);
    if (mapItem)
        return addMapItem_real(mapItem);

    return false;
}

bool QDeclarativeGeoMap::removeMapChild(QObject *child)
{
    // dispatch items appropriately
    QDeclarativeGeoMapItemView *mapView = qobject_cast<QDeclarativeGeoMapItemView *>(child);
    if (mapView)
        return removeMapItemView_real(mapView);

    QDeclarativeGeoMapItemGroup *itemGroup = qobject_cast<QDeclarativeGeoMapItemGroup *>(child);
    if (itemGroup) // removeMapItemView calls removeMapItemGroup for itself.
        return removeMapItemGroup_real(itemGroup);

    QDeclarativeGeoMapItemBase *mapItem = qobject_cast<QDeclarativeGeoMapItemBase *>(child);
    if (mapItem)
        return removeMapItem_real(mapItem);

    return false;
}

bool QDeclarativeGeoMap::isGroupNested(QDeclarativeGeoMapItemGroup *group) const
{
    QObject *parent = group->parent();
    // Nested groups have parent set in parent's componentComplete()
    // Those instantiated by MapItemView's delegateModel, however, do not,
    // but have setParentItem set.
    return qobject_cast<QDeclarativeGeoMapItemGroup *>(parent)
            || qobject_cast<QDeclarativeGeoMapItemGroup *>(group->parentItem());
}

qreal QDeclarativeGeoMap::zoomLevel() const
{
    if (m_initialized)
        return m_map->cameraData().zoomLevel();
    return m_cameraData.zoomLevel();
}

/*!
    \qmlproperty real QtLocation::Map::bearing

    This property holds the bearing for the map.
    The default value is 0.
    If the Plugin used for the Map supports bearing, the valid range for this value is between 0 and 360.
    If the Plugin used for the Map does not support bearing, changing this property will have no effect.

    \since QtLocation 5.9
*/
void QDeclarativeGeoMap::setBearing(qreal bearing)
{
    bearing = sanitizeBearing(bearing);
    if (m_initialized) {
        QGeoCameraData cameraData = m_map->cameraData();
        cameraData.setBearing(bearing);
        m_map->setCameraData(cameraData);
    } else {
        const bool bearingHasChanged = bearing != m_cameraData.bearing();
        m_cameraData.setBearing(bearing);
        if (bearingHasChanged) {
            emit bearingChanged(bearing);
            // do not emit visibleRegionChanged() here, because, if the map isn't initialized,
            // the getter won't return anything updated
        }
    }
}

/*!
    \qmlmethod void QtLocation::Map::setBearing(real bearing, coordinate coordinate)

    Sets the bearing for the map to \a bearing, rotating it around \a coordinate.
    If the Plugin used for the Map supports bearing, the valid range for \a bearing is between 0 and 360.
    If the Plugin used for the Map does not support bearing, or if the map is tilted and \a coordinate happens
    to be behind the camera, or if the map is not ready (see \l mapReady), calling this method will have no effect.

    The release of this API with Qt 5.10 is a Technology Preview.

    \since 5.10
*/
void QDeclarativeGeoMap::setBearing(qreal bearing, const QGeoCoordinate &coordinate)
{
    if (!m_initialized)
        return;

    const QGeoCoordinate currentCenter = center();
    const qreal currentBearing = QDeclarativeGeoMap::bearing();
    bearing = sanitizeBearing(bearing);

    if (!coordinate.isValid()
            || !qIsFinite(bearing)
            || (coordinate == currentCenter && bearing == currentBearing))
        return;

    if (m_map->capabilities() & QGeoMap::SupportsSetBearing)
        m_map->setBearing(bearing, coordinate);
}

qreal QDeclarativeGeoMap::bearing() const
{
    if (m_initialized)
        return m_map->cameraData().bearing();
    return m_cameraData.bearing();
}

/*!
    \qmlproperty real QtLocation::Map::tilt

    This property holds the tilt for the map, in degrees.
    The default value is 0.
    The valid range for this value is [ minimumTilt, maximumTilt ].
    If the Plugin used for the Map does not support tilting, changing this property will have no effect.

    \sa minimumTilt, maximumTilt

    \since QtLocation 5.9
*/
void QDeclarativeGeoMap::setTilt(qreal tilt)
{
    tilt = qBound(minimumTilt(), tilt, maximumTilt());

    if (m_initialized) {
        QGeoCameraData cameraData = m_map->cameraData();
        cameraData.setTilt(tilt);
        m_map->setCameraData(cameraData);
    } else {
        const bool tiltHasChanged = tilt != m_cameraData.tilt();
        m_cameraData.setTilt(tilt);
        if (tiltHasChanged) {
            emit tiltChanged(tilt);
            // do not emit visibleRegionChanged() here, because, if the map isn't initialized,
            // the getter won't return anything updated
        }
    }
}

qreal QDeclarativeGeoMap::tilt() const
{
    if (m_initialized)
        return m_map->cameraData().tilt();
    return m_cameraData.tilt();
}

void QDeclarativeGeoMap::setMinimumTilt(qreal minimumTilt, bool userSet)
{
    if (minimumTilt >= 0) {
        qreal oldMinimumTilt = this->minimumTilt();

        if (userSet)
            m_userMinimumTilt = minimumTilt;
        else
            m_minimumTilt = minimumTilt;

        if (tilt() < minimumTilt)
            setTilt(minimumTilt);

        if (oldMinimumTilt != this->minimumTilt())
            emit minimumTiltChanged(this->minimumTilt());
    }
}

/*!
    \qmlproperty real QtLocation::Map::fieldOfView

    This property holds the field of view of the camera used to look at the map, in degrees.
    If the plugin property of the map is not set, or the plugin does not support mapping, the value is 45 degrees.

    Note that changing this value implicitly changes also the distance between the camera and the map,
    so that, at a tilting angle of 0 degrees, the resulting image is identical for any value used for this property.

    For more information about this parameter, consult the Wikipedia articles about \l {https://en.wikipedia.org/wiki/Field_of_view} {Field of view} and
    \l {https://en.wikipedia.org/wiki/Angle_of_view} {Angle of view}.

    \sa minimumFieldOfView, maximumFieldOfView

    \since QtLocation 5.9
*/
void QDeclarativeGeoMap::setFieldOfView(qreal fieldOfView)
{
    fieldOfView = qBound(minimumFieldOfView(), fieldOfView, maximumFieldOfView());

    if (m_initialized) {
        QGeoCameraData cameraData = m_map->cameraData();
        cameraData.setFieldOfView(fieldOfView);
        m_map->setCameraData(cameraData);
    } else {
        const bool fovChanged = fieldOfView != m_cameraData.fieldOfView();
        m_cameraData.setFieldOfView(fieldOfView);
        if (fovChanged) {
            emit fieldOfViewChanged(fieldOfView);
            // do not emit visibleRegionChanged() here, because, if the map isn't initialized,
            // the getter won't return anything updated
        }
    }
}

qreal QDeclarativeGeoMap::fieldOfView() const
{
    if (m_initialized)
        return m_map->cameraData().fieldOfView();
    return m_cameraData.fieldOfView();
}

void QDeclarativeGeoMap::setMinimumFieldOfView(qreal minimumFieldOfView, bool userSet)
{
    if (minimumFieldOfView > 0 && minimumFieldOfView < 180.0) {
        qreal oldMinimumFoV = this->minimumFieldOfView();

        if (userSet)
            m_userMinimumFieldOfView = minimumFieldOfView;
        else
            m_minimumFieldOfView = minimumFieldOfView;

        if (fieldOfView() < minimumFieldOfView)
            setFieldOfView(minimumFieldOfView);

        if (oldMinimumFoV != this->minimumFieldOfView())
            emit minimumFieldOfViewChanged(this->minimumFieldOfView());
    }
}

/*!
    \qmlproperty real QtLocation::Map::minimumFieldOfView

    This property holds the minimum valid field of view for the map, in degrees.

    The minimum tilt field of view by the \l plugin used is a lower bound for
    this property.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 1.

    \sa fieldOfView, maximumFieldOfView

    \since QtLocation 5.9
*/
qreal QDeclarativeGeoMap::minimumFieldOfView() const
{
    return qMax(qMin(m_maximumFieldOfView, m_userMinimumFieldOfView), m_minimumFieldOfView);
}

void QDeclarativeGeoMap::setMaximumFieldOfView(qreal maximumFieldOfView, bool userSet)
{
    if (maximumFieldOfView > 0 && maximumFieldOfView < 180.0) {
        qreal oldMaximumFoV = this->maximumFieldOfView();
        if (userSet)
            m_userMaximumFieldOfView = maximumFieldOfView;
        else
            m_maximumFieldOfView = maximumFieldOfView;

        if (fieldOfView() > maximumFieldOfView)
            setFieldOfView(maximumFieldOfView);

        if (oldMaximumFoV != this->maximumFieldOfView())
            emit maximumFieldOfViewChanged(this->maximumFieldOfView());
    }
}

/*!
    \qmlproperty real QtLocation::Map::maximumFieldOfView

    This property holds the maximum valid field of view for the map, in degrees.

    The minimum tilt field of view by the \l plugin used is an upper bound for
    this property.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 179.

    \sa fieldOfView, minimumFieldOfView

    \since QtLocation 5.9
*/
qreal QDeclarativeGeoMap::maximumFieldOfView() const
{
    return qMin(qMax(m_minimumFieldOfView, m_userMaximumFieldOfView), m_maximumFieldOfView);
}

/*!
    \qmlproperty real QtLocation::Map::minimumTilt

    This property holds the minimum valid tilt for the map, in degrees.

    The minimum tilt defined by the \l plugin used is a lower bound for
    this property.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 0.

    Since QtLocation 5.12, plugins can additionally restrict this value depending on the current zoom level.

    \sa tilt, maximumTilt

    \since QtLocation 5.9
*/
qreal QDeclarativeGeoMap::minimumTilt() const
{
    return qMax(qMin(m_maximumTilt, m_userMinimumTilt), m_minimumTilt);
}

void QDeclarativeGeoMap::setMaximumTilt(qreal maximumTilt, bool userSet)
{
    if (maximumTilt >= 0) {
        qreal oldMaximumTilt = this->maximumTilt();

        if (userSet)
            m_userMaximumTilt = maximumTilt;
        else
            m_maximumTilt = maximumTilt;

        if (tilt() > maximumTilt)
            setTilt(maximumTilt);

        if (oldMaximumTilt != this->maximumTilt())
            emit maximumTiltChanged(this->maximumTilt());
    }
}

/*!
    \qmlproperty real QtLocation::Map::maximumTilt

    This property holds the maximum valid tilt for the map, in degrees.

    The maximum tilt defined by the \l plugin used is an upper bound for
    this property.
    If the \l plugin property is not set or the plugin does not support mapping, this property is \c 89.5.

    Since QtLocation 5.12, plugins can additionally restrict this value depending on the current zoom level.

    \sa tilt, minimumTilt

    \since QtLocation 5.9
*/
qreal QDeclarativeGeoMap::maximumTilt() const
{
    return qMin(qMax(m_minimumTilt, m_userMaximumTilt), m_maximumTilt);
}

/*!
    \qmlproperty coordinate QtLocation::Map::center

    This property holds the coordinate which occupies the center of the
    mapping viewport. Invalid center coordinates are ignored.

    The default value is an arbitrary valid coordinate.
*/
void QDeclarativeGeoMap::setCenter(const QGeoCoordinate &center)
{
    if (!center.isValid())
        return;

    if (m_initialized) {
        QGeoCoordinate coord(center);
        coord.setLatitude(qBound(m_minimumViewportLatitude, center.latitude(), m_maximumViewportLatitude));
        QGeoCameraData cameraData = m_map->cameraData();
        cameraData.setCenter(coord);
        m_map->setCameraData(cameraData);
    } else {
        const bool centerHasChanged = center != m_cameraData.center();
        m_cameraData.setCenter(center);
        if (centerHasChanged) {
            emit centerChanged(center);
            // do not emit visibleRegionChanged() here, because, if the map isn't initialized,
            // the getter won't return anything updated
        }
    }
}

QGeoCoordinate QDeclarativeGeoMap::center() const
{
    if (m_initialized)
        return m_map->cameraData().center();
    return m_cameraData.center();
}


/*!
    \qmlproperty geoShape QtLocation::Map::visibleRegion

    This property holds the region which occupies the viewport of
    the map. The camera is positioned in the center of the shape, and
    at the largest integral zoom level possible which allows the
    whole shape to be visible on the screen. This implies that
    reading this property back shortly after having been set the
    returned area is equal or larger than the set area.

    Setting this property implicitly changes the \l center and
    \l zoomLevel of the map. Any previously set value to those
    properties will be overridden.

    \note Since Qt 5.14 This property provides change notifications.

    \since 5.6
*/
void QDeclarativeGeoMap::setVisibleRegion(const QGeoShape &shape)
{
    if (shape.boundingGeoRectangle() == visibleRegion())
        return;

    m_visibleRegion = shape.boundingGeoRectangle();
    if (!m_visibleRegion.isValid()
        || (m_visibleRegion.bottomRight().latitude() >= 85.0) // rect entirely outside web mercator
        || (m_visibleRegion.topLeft().latitude() <= -85.0)) {
        // shape invalidated -> nothing to fit anymore
        m_visibleRegion = QGeoRectangle();
        m_pendingFitViewport = false;
        emit visibleRegionChanged();
        return;
    }

    if (!m_map || !width() || !height()) {
        m_pendingFitViewport = true;
        emit visibleRegionChanged();
        return;
    }

    fitViewportToGeoShape(m_visibleRegion);
    emit visibleRegionChanged();
}

QGeoShape QDeclarativeGeoMap::visibleRegion() const
{
    if (!m_map || !width() || !height())
        return m_visibleRegion;

    if (m_map->capabilities() & QGeoMap::SupportsVisibleRegion) {
        return m_map->visibleRegion();
    } else {
        // ToDo: handle projections not supporting visible region in a better way.
        // This approach will fail when horizon is in the view or the map is greatly zoomed out.
        QList<QGeoCoordinate> visiblePoly;
        visiblePoly << m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(0,0), false);
        visiblePoly << m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(m_map->viewportWidth() - 1,
                                                                                       0), false);
        visiblePoly << m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(m_map->viewportWidth() - 1,
                                                                                       m_map->viewportHeight() - 1), false);
        visiblePoly << m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(0,
                                                                                       m_map->viewportHeight() - 1), false);
        QGeoPath path;
        path.setPath(visiblePoly);
        return path.boundingGeoRectangle();
    }
}

/*!
    \qmlproperty bool QtLocation::Map::copyrightsVisible

    This property holds the visibility of the copyrights notice. The notice is usually
    displayed in the bottom left corner. By default, this property is set to \c true.

    \note Many map providers require the notice to be visible as part of the terms and conditions.
    Please consult the relevant provider documentation before turning this notice off.

    \since 5.7
*/
void QDeclarativeGeoMap::setCopyrightsVisible(bool visible)
{
    if (m_copyrightsVisible == visible)
        return;

    if (!m_copyrights.isNull())
        m_copyrights->setCopyrightsVisible(visible);

    m_copyrightsVisible = visible;
    emit copyrightsVisibleChanged(visible);
}

bool QDeclarativeGeoMap::copyrightsVisible() const
{
    return m_copyrightsVisible;
}



/*!
    \qmlproperty color QtLocation::Map::color

    This property holds the background color of the map element.

    \since 5.6
*/
void QDeclarativeGeoMap::setColor(const QColor &color)
{
    if (color != m_color) {
        m_color = color;
        update();
        emit colorChanged(m_color);
    }
}

QColor QDeclarativeGeoMap::color() const
{
    return m_color;
}

/*!
    \qmlproperty rect QtLocation::Map::visibleArea

    This property holds the visible area inside the Map QML element.
    It is a rect whose coordinates are relative to the Map element.
    Its size will be clamped to the size of the Map element.
    A null visibleArea means that the whole Map is visible.

    \since 5.12
*/
QRectF QDeclarativeGeoMap::visibleArea() const
{
    if (m_initialized)
        return m_map->visibleArea();
    return m_visibleArea;
}

void QDeclarativeGeoMap::setVisibleArea(const QRectF &visibleArea)
{
    const QRectF oldVisibleArea = QDeclarativeGeoMap::visibleArea();
    if (visibleArea == oldVisibleArea)
        return;

    if (!visibleArea.isValid() && !visibleArea.isEmpty()) // values < 0
        return;

    if (m_initialized) {
        m_map->setVisibleArea(visibleArea);
        const QRectF newVisibleArea = QDeclarativeGeoMap::visibleArea();
        if (newVisibleArea != oldVisibleArea) {
            // polish map items
            for (const QPointer<QDeclarativeGeoMapItemBase> &i: std::as_const(m_mapItems)) {
                if (i)
                    i->visibleAreaChanged();
            }
        }
    } else {
        m_visibleArea = visibleArea;
        const QRectF newVisibleArea = QDeclarativeGeoMap::visibleArea();
        if (newVisibleArea != oldVisibleArea)
            emit visibleAreaChanged();
    }
}

/*!
    \qmlproperty bool QtLocation::Map::mapReady

    This property holds whether the map has been successfully initialized and is ready to be used.
    Some methods, such as \l fromCoordinate and \l toCoordinate, will not work before the map is ready.
    Due to the architecture of the \l Map, it's advised to use the signal emitted for this property
    in place of \l {QtQml::Component::completed()}{Component.onCompleted}, to make sure that everything behaves as expected.

    \since 5.9
*/
bool QDeclarativeGeoMap::mapReady() const
{
    return m_initialized;
}

QMargins QDeclarativeGeoMap::mapMargins() const
{
    const QRectF va = m_map->visibleArea();
    if (va.isEmpty())
        return QMargins();
    return QMargins( va.x()
                    , va.y()
                    , width() - va.width() - va.x()
                    , height() - va.height() - va.y());
}

/*!
    \qmlproperty list<mapType> QtLocation::Map::supportedMapTypes

    This read-only property holds the set of \l{mapType}{map types} supported by this map.

    \sa activeMapType
*/
QList<QGeoMapType> QDeclarativeGeoMap::supportedMapTypes()
{
    return m_supportedMapTypes;
}

/*!
    \qmlmethod void QtLocation::Map::alignCoordinateToPoint(coordinate coordinate, QPointF point)

    Aligns \a coordinate to \a point.
    This method effectively extends the functionality offered by the \l center qml property, allowing
    to align a coordinate to point of the Map element other than its center.
    This is useful in those applications where the center of the scene (e.g., a cursor) is not to be
    placed exactly in the center of the map.

    If the map is tilted, and \a coordinate happens to be behind the camera, or if the map is not ready
    (see \l mapReady), calling this method will have no effect.

    The release of this API with Qt 5.10 is a Technology Preview.

    \sa center

    \since 5.10
*/
void QDeclarativeGeoMap::alignCoordinateToPoint(const QGeoCoordinate &coordinate, const QPointF &point)
{
    if (!m_map || !(m_map->capabilities() & QGeoMap::SupportsAnchoringCoordinate))
        return;

    if (!coordinate.isValid()
            || !qIsFinite(point.x())
            || !qIsFinite(point.y()))
        return;

    m_map->anchorCoordinateToPoint(coordinate, point);
}

/*!
    \qmlmethod coordinate QtLocation::Map::toCoordinate(QPointF position, bool clipToViewPort)

    Returns the coordinate which corresponds to the \a position relative to the map item.

    If \a clipToViewPort is \c true, or not supplied then returns an invalid coordinate if
    \a position is not within the current viewport.
*/
QGeoCoordinate QDeclarativeGeoMap::toCoordinate(const QPointF &position, bool clipToViewPort) const
{
    if (m_map)
        return m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(position), clipToViewPort);
    else
        return QGeoCoordinate();
}

/*!
    \qmlmethod point QtLocation::Map::fromCoordinate(coordinate coordinate, bool clipToViewPort)

    Returns the position relative to the map item which corresponds to the \a coordinate.

    If \a clipToViewPort is \c true, or not supplied then returns an invalid QPointF if
    \a coordinate is not within the current viewport.
*/
QPointF QDeclarativeGeoMap::fromCoordinate(const QGeoCoordinate &coordinate, bool clipToViewPort) const
{
    if (m_map)
        return m_map->geoProjection().coordinateToItemPosition(coordinate, clipToViewPort).toPointF();
    else
        return QPointF(qQNaN(), qQNaN());
}

/*!
    \qmlmethod void QtLocation::Map::pan(int dx, int dy)

    Starts panning the map by \a dx pixels along the x-axis and
    by \a dy pixels along the y-axis.

    Positive values for \a dx move the map right, negative values left.
    Positive values for \a dy move the map down, negative values up.

    During panning the \l center, and \l zoomLevel may change.
*/
void QDeclarativeGeoMap::pan(int dx, int dy)
{
    if (!m_map)
        return;
    if (dx == 0 && dy == 0)
        return;

    QGeoCoordinate coord = m_map->geoProjection().itemPositionToCoordinate(
                                QDoubleVector2D(m_map->viewportWidth() / 2 + dx,
                                        m_map->viewportHeight() / 2 + dy));
    setCenter(coord);
}


/*!
    \qmlmethod void QtLocation::Map::prefetchData()

    Optional hint that allows the map to prefetch during this idle period
*/
void QDeclarativeGeoMap::prefetchData()
{
    if (!m_map)
        return;
    m_map->prefetchData();
}

/*!
    \qmlmethod void QtLocation::Map::clearData()

    Clears map data collected by the currently selected plugin.
    \note This method will delete cached files.
    \sa plugin
*/
void QDeclarativeGeoMap::clearData()
{
    if (m_map)
        m_map->clearData();
}

/*!
    \qmlmethod void QtLocation::Map::fitViewportToGeoShape(geoShape, margins)

    Fits the viewport to a specific geo shape \a geoShape.
    The \a margins are in screen pixels.

    \note If the projection used by the plugin is not WebMercator, and the plugin does not have fitting to
    shape capability, this method will do nothing.

    \sa visibleRegion
    \since 5.13
*/
void QDeclarativeGeoMap::fitViewportToGeoShape(const QGeoShape &shape, QVariant margins)
{
    QMargins m(10, 10, 10, 10); // lets defaults to 10 if margins is invalid
    switch (margins.typeId()) {
        case QMetaType::Int:
        case QMetaType::Double: {
            const int value = int(margins.toDouble());
            m = QMargins(value, value, value, value);
        }
        break;
        // ToDo: Support distinct margins in some QML form. Perhaps QRect?
        default:
            break;
    }
    fitViewportToGeoShape(shape, m);
}

void QDeclarativeGeoMap::fitViewportToGeoShape(const QGeoShape &shape, const QMargins &borders)
{
    if (!m_map  || !shape.isValid())
        return;

    if (m_map->geoProjection().projectionType() == QGeoProjection::ProjectionWebMercator) {
        // This case remains handled here, and not inside QGeoMap*::fitViewportToGeoRectangle,
        // in order to honor animations on center and zoomLevel
        const QMargins margins = borders + mapMargins();
        const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(m_map->geoProjection());
        const QPair<QGeoCoordinate, qreal> fitData = p.fitViewportToGeoRectangle(shape.boundingGeoRectangle(),
                                                                                 margins);
        if (!fitData.first.isValid())
            return;

        // position camera to the center of bounding box
        setProperty("center", QVariant::fromValue(fitData.first)); // not using setCenter(centerCoordinate) to honor a possible animation set on the center property

        if (!qIsFinite(fitData.second))
            return;
        double newZoom = qMax<double>(minimumZoomLevel(), fitData.second);
        setProperty("zoomLevel", QVariant::fromValue(newZoom)); // not using setZoomLevel(newZoom)  to honor a possible animation set on the zoomLevel property
    } else if (m_map->capabilities() & QGeoMap::SupportsFittingViewportToGeoRectangle) {
        // Animations cannot be honored in this case, as m_map acts as a black box
        m_map->fitViewportToGeoRectangle(m_visibleRegion, borders);
    }
    // else out of luck
}

/*!
    \qmlproperty string QtLocation::Map::errorString

    This read-only property holds the textual presentation of the latest mapping provider error.
    If no error has occurred, an empty string is returned.

    An empty string may also be returned if an error occurred which has no associated
    textual representation.

    \sa QGeoServiceProvider::errorString()
*/

QString QDeclarativeGeoMap::errorString() const
{
    return m_errorString;
}

/*!
    \qmlproperty enumeration QtLocation::Map::error

    This read-only property holds the last occurred mapping service provider error.

    \list
    \li Map.NoError - No error has occurred.
    \li Map.NotSupportedError -The maps plugin property was not set or there is no mapping manager associated with the plugin.
    \li Map.UnknownParameterError -The plugin did not recognize one of the parameters it was given.
    \li Map.MissingRequiredParameterError - The plugin did not find one of the parameters it was expecting.
    \li Map.ConnectionError - The plugin could not connect to its backend service or database.
    \endlist

    \sa QGeoServiceProvider::Error
*/

QGeoServiceProvider::Error QDeclarativeGeoMap::error() const
{
    return m_error;
}

QGeoMap *QDeclarativeGeoMap::map() const
{
    return m_map;
}

void QDeclarativeGeoMap::itemChange(ItemChange change, const ItemChangeData &value)
{
    if (change == ItemChildAddedChange) {
        QQuickItem *child = value.item;
        QQuickItem *mapItem = qobject_cast<QDeclarativeGeoMapItemBase *>(child);
        if (!mapItem)
            mapItem = qobject_cast<QDeclarativeGeoMapItemGroup *>(child);

        if (mapItem) {
            qreal z = mapItem->z();
            if (z > m_maxChildZ) { // Ignore children removal
                m_maxChildZ = z;
                // put the copyrights notice object at the highest z order
                if (m_copyrights)
                    m_copyrights->setCopyrightsZ(m_maxChildZ + 1);
            }
        }
    } else if (change == ItemSceneChange) {
        if (m_window) {
            disconnect(m_window, &QQuickWindow::beforeSynchronizing,
                       this, &QDeclarativeGeoMap::updateItemToWindowTransform);
        }
        m_window = value.window;
        if (m_window) {
            connect(m_window, &QQuickWindow::beforeSynchronizing,
                    this, &QDeclarativeGeoMap::updateItemToWindowTransform, Qt::DirectConnection);
        }
    }
    QQuickItem::itemChange(change, value);
}

void QDeclarativeGeoMap::attachCopyrightNotice(bool initialVisibility)
{
    if (initialVisibility) {
        ++m_copyNoticesVisible;
        if (m_map)
            m_map->setCopyrightVisible(m_copyNoticesVisible > 0);
    }
}

void QDeclarativeGeoMap::detachCopyrightNotice(bool currentVisibility)
{
    if (currentVisibility) {
        --m_copyNoticesVisible;
        if (m_map)
            m_map->setCopyrightVisible(m_copyNoticesVisible > 0);
    }
}

void QDeclarativeGeoMap::onAttachedCopyrightNoticeVisibilityChanged()
{
    QDeclarativeGeoMapCopyrightNotice *copy = static_cast<QDeclarativeGeoMapCopyrightNotice *>(sender());
    m_copyNoticesVisible += ( int(copy->copyrightsVisible()) * 2 - 1);
    if (m_map)
        m_map->setCopyrightVisible(m_copyNoticesVisible > 0);
}

void QDeclarativeGeoMap::onCameraDataChanged(const QGeoCameraData &cameraData)
{
    bool centerHasChanged = cameraData.center() != m_cameraData.center();
    bool bearingHasChanged = cameraData.bearing() != m_cameraData.bearing();
    bool tiltHasChanged = cameraData.tilt() != m_cameraData.tilt();
    bool fovHasChanged = cameraData.fieldOfView() != m_cameraData.fieldOfView();
    bool zoomHasChanged = cameraData.zoomLevel() != m_cameraData.zoomLevel();

    m_cameraData = cameraData;
    // polish map items
    for (const QPointer<QDeclarativeGeoMapItemBase> &i: std::as_const(m_mapItems)) {
        if (i)
            i->baseCameraDataChanged(m_cameraData); // Consider optimizing this further, removing the contained duplicate if conditions.
    }

    if (centerHasChanged)
        emit centerChanged(m_cameraData.center());
    if (zoomHasChanged)
        emit zoomLevelChanged(m_cameraData.zoomLevel());
    if (bearingHasChanged)
        emit bearingChanged(m_cameraData.bearing());
    if (tiltHasChanged)
        emit tiltChanged(m_cameraData.tilt());
    if (fovHasChanged)
        emit fieldOfViewChanged(m_cameraData.fieldOfView());
    if (centerHasChanged || zoomHasChanged || bearingHasChanged
            || tiltHasChanged || fovHasChanged)
        emit visibleRegionChanged();
}

/*!
    \qmlproperty list<MapItem> QtLocation::Map::mapItems

    Returns the list of all map items in no particular order.
    These items include items that were declared statically as part of
    the type declaration, as well as dynamical items (\l addMapItem,
    \l MapItemView).

    \sa addMapItem, removeMapItem, clearMapItems
*/

QList<QObject *> QDeclarativeGeoMap::mapItems()
{
    QList<QObject *> ret;
    for (const auto &ptr : m_mapItems) {
        if (ptr)
            ret << ptr.data();
    }
    return ret;
}

/*!
    \qmlmethod void QtLocation::Map::addMapItem(MapItem item)

    Adds the given \a item to the Map (for example MapQuickItem, MapCircle). If the object
    already is on the Map, it will not be added again.

    As an example, consider the case where you have a MapCircle representing your current position:

    \snippet declarative/maps.qml QtQuick import
    \snippet declarative/maps.qml QtLocation import
    \codeline
    \snippet declarative/maps.qml Map addMapItem MapCircle at current position

    \note MapItemViews cannot be added with this method.

    \sa mapItems, removeMapItem, clearMapItems
*/

void QDeclarativeGeoMap::addMapItem(QDeclarativeGeoMapItemBase *item)
{
    if (addMapItem_real(item))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::addMapItem_real(QDeclarativeGeoMapItemBase *item)
{
    if (!item || item->quickMap())
        return false;
    // If the item comes from a MapItemGroup, do not reparent it.
    if (!qobject_cast<QDeclarativeGeoMapItemGroup *>(item->parentItem()))
        item->setParentItem(this);
    m_mapItems.append(item);
    if (m_map) {
        item->setMap(this, m_map);
        m_map->addMapItem(item);
    }
    return true;
}

/*!
    \qmlmethod void QtLocation::Map::removeMapItem(MapItem item)

    Removes the given \a item from the Map (for example MapQuickItem, MapCircle). If
    the MapItem does not exist or was not previously added to the map, the
    method does nothing.

    \sa mapItems, addMapItem, clearMapItems
*/
void QDeclarativeGeoMap::removeMapItem(QDeclarativeGeoMapItemBase *ptr)
{
    if (removeMapItem_real(ptr))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::removeMapItem_real(QDeclarativeGeoMapItemBase *ptr)
{
    if (!ptr)
        return false;
    QPointer<QDeclarativeGeoMapItemBase> item(ptr);
    if (!m_mapItems.contains(item))
        return false;
    if (m_map)
        m_map->removeMapItem(ptr);
    if (item->parentItem() == this)
        item->setParentItem(0);
    item->setMap(0, 0);
    // these can be optimized for perf, as we already check the 'contains' above
    m_mapItems.removeOne(item);
    return true;
}

/*!
    \qmlmethod void QtLocation::Map::clearMapItems()

    Removes all items and item groups from the map.

    \sa mapItems, addMapItem, removeMapItem, addMapItemGroup, removeMapItemGroup
*/
void QDeclarativeGeoMap::clearMapItems()
{
    if (m_mapItems.isEmpty())
        return;

    qsizetype removed = 0;
    for (qsizetype i = 0; i < m_mapItemGroups.count(); ++i) {
        auto item = m_mapItemGroups.at(i);
        // Processing only top-level groups (!views)
        if (qobject_cast<QDeclarativeGeoMapItemView *>(item))
            continue;


        if (item->parentItem() != this)
            continue;

        if (removeMapItemGroup_real(item)) {
            removed++;
            --i;
        }
    }

    while (!m_mapItems.isEmpty())
        removed += removeMapItem_real(m_mapItems.first());

    if (removed)
        emit mapItemsChanged();
}

/*!
    \qmlmethod void QtLocation::Map::addMapItemGroup(MapItemGroup itemGroup)

    Adds the map items contained in the given \a itemGroup to the Map
    (for example MapQuickItem, MapCircle).

    \sa MapItemGroup, removeMapItemGroup

    \since 5.9
*/
void QDeclarativeGeoMap::addMapItemGroup(QDeclarativeGeoMapItemGroup *itemGroup)
{
    if (addMapItemGroup_real(itemGroup))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::addMapItemGroup_real(QDeclarativeGeoMapItemGroup *itemGroup)
{
    if (!itemGroup || itemGroup->quickMap()) // Already added to some map
        return false;

    itemGroup->setQuickMap(this);

    if (!isGroupNested(itemGroup))
        itemGroup->setParentItem(this);

    QPointer<QDeclarativeGeoMapItemGroup> g(itemGroup);
    m_mapItemGroups.append(g);

    const QList<QQuickItem *> quickKids = itemGroup->childItems();
    int count = 0;
    for (auto c: quickKids) {
        count += addMapChild(c); // this calls addMapItemGroup recursively, if needed
    }
    return count;
}

/*!
    \qmlmethod void QtLocation::Map::removeMapItemGroup(MapItemGroup itemGroup)

    Removes \a itemGroup and the items contained therein from the Map.

    \sa MapItemGroup, addMapItemGroup

    \since 5.9
*/
void QDeclarativeGeoMap::removeMapItemGroup(QDeclarativeGeoMapItemGroup *itemGroup)
{
    if (removeMapItemGroup_real(itemGroup))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::removeMapItemGroup_real(QDeclarativeGeoMapItemGroup *itemGroup)
{
    if (!itemGroup || itemGroup->quickMap() != this) // cant remove an itemGroup added to another map
        return false;

    QPointer<QDeclarativeGeoMapItemGroup> g(itemGroup);
    if (!m_mapItemGroups.removeOne(g))
        return false;

    const QList<QQuickItem *> quickKids = itemGroup->childItems();
    int count = 0;
    for (auto c: quickKids) {
        count += removeMapChild(c);
    }
    itemGroup->setQuickMap(nullptr);
    if (itemGroup->parentItem() == this)
        itemGroup->setParentItem(0);
    return count;
}

/*!
    \qmlmethod void QtLocation::Map::removeMapItemView(MapItemView itemView)

    Removes \a itemView and the items instantiated by it from the Map.

    \sa MapItemView, addMapItemView

    \since 5.10
*/
void QDeclarativeGeoMap::removeMapItemView(QDeclarativeGeoMapItemView *itemView)
{
    if (removeMapItemView_real(itemView))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::removeMapItemView_real(QDeclarativeGeoMapItemView *itemView)
{
    if (!itemView || itemView->m_map != this) // can't remove a view that is already added to another map
        return false;

    itemView->removeInstantiatedItems(false); // remove the items without using transitions AND abort ongoing ones
    itemView->m_map = 0;
    m_mapViews.removeOne(itemView);
    return removeMapItemGroup_real(itemView); // at this point, all delegate instances have been removed.
}

void QDeclarativeGeoMap::updateItemToWindowTransform()
{
    if (!m_initialized)
        return;

    // Update itemToWindowTransform into QGeoProjection
    QTransform item2Window = QQuickItemPrivate::get(this)->itemToWindowTransform();
    if (!property("layer").isNull() && property("layer").value<QObject *>()->property("enabled").toBool())
        item2Window.reset(); // When layer is enabled, the item is rendered offscreen with no transformation, then the layer is applied

    m_map->setItemToWindowTransform(item2Window);

    m_sgNodeHasChanged = false;
}

void QDeclarativeGeoMap::onSGNodeChanged()
{
    m_sgNodeHasChanged = true;
    update();
}

/*!
    \qmlmethod void QtLocation::Map::addMapItemView(MapItemView itemView)

    Adds \a itemView to the Map.

    \sa MapItemView, removeMapItemView

    \since 5.10
*/
void QDeclarativeGeoMap::addMapItemView(QDeclarativeGeoMapItemView *itemView)
{
    if (addMapItemView_real(itemView))
        emit mapItemsChanged();
}

bool QDeclarativeGeoMap::addMapItemView_real(QDeclarativeGeoMapItemView *itemView)
{
    if (!itemView || itemView->m_map) // can't add a view twice
        return false;

    int count = addMapItemGroup_real(itemView); // at this point, delegates aren't yet incubated.
    // Not appending it to m_mapViews because it seems unnecessary even if the
    // itemView is a child of this (in which case it would be destroyed
    m_mapViews.append(itemView);
    setupMapView(itemView);
    return count;
}

/*!
    \qmlproperty mapType QtLocation::Map::activeMapType

    \brief Access to the currently active \l{mapType}{map type}.

    This property can be set to change the active \l{mapType}{map type}.
    See the \l{Map::supportedMapTypes}{supportedMapTypes} property for possible values.

    \sa mapType
*/
void QDeclarativeGeoMap::setActiveMapType(const QGeoMapType &mapType)
{
    if (m_activeMapType != mapType) {
        if (m_map) {
            if (mapType.pluginName() == m_plugin->name().toLatin1()) {
                m_map->setActiveMapType(mapType);
                m_activeMapType = mapType;
                emit activeMapTypeChanged();
            }
        } else {
            m_activeMapType = mapType;
            emit activeMapTypeChanged();
        }
    }
}

QGeoMapType QDeclarativeGeoMap::activeMapType() const
{
    return m_activeMapType;
}

/*!
    \internal
*/
void QDeclarativeGeoMap::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);

    if (!m_map || newGeometry.size().isEmpty())
        return;

    m_map->setViewportSize(newGeometry.size().toSize());

    if (!m_initialized) {
        initialize();
    } else {
        setMinimumZoomLevel(m_map->minimumZoom(), false);

        // Update the center latitudinal threshold
        QGeoCameraData cameraData = m_map->cameraData();
        const double maximumCenterLatitudeAtZoom = m_map->maximumCenterLatitudeAtZoom(cameraData);
        const double minimumCenterLatitudeAtZoom = m_map->minimumCenterLatitudeAtZoom(cameraData);
        if (maximumCenterLatitudeAtZoom != m_maximumViewportLatitude
                || minimumCenterLatitudeAtZoom != m_minimumViewportLatitude) {
            m_maximumViewportLatitude = maximumCenterLatitudeAtZoom;
            m_minimumViewportLatitude = minimumCenterLatitudeAtZoom;
            QGeoCoordinate coord = cameraData.center();
            coord.setLatitude(qBound(m_minimumViewportLatitude, coord.latitude(), m_maximumViewportLatitude));
            cameraData.setCenter(coord);
            m_map->setCameraData(cameraData);
        }
        if (oldGeometry.size() != newGeometry.size()) {
            // polish map items
            for (const QPointer<QDeclarativeGeoMapItemBase> &i: std::as_const(m_mapItems)) {
                if (i)
                    i->polishAndUpdate();
            }
        }
    }

    /*
        The fitViewportTo*() functions depend on a valid map geometry.
        If they were called prior to the first resize they cause
        the zoomlevel to jump to 0 (showing the world). Therefore the
        calls were queued up until now.

        Multiple fitViewportTo*() calls replace each other.
     */
    if (m_pendingFitViewport && width() && height()) {
        fitViewportToGeoShape(m_visibleRegion);
        m_pendingFitViewport = false;
    }

}

/*!
    \qmlmethod void QtLocation::Map::fitViewportToMapItems(list<MapItems> items = {})

    If no argument is provided, fits the current viewport to the boundary of all map items.
    The camera is positioned in the center of the map items, and at the largest integral zoom level
    possible which allows all map items to be visible on screen.
    If \a items is provided, fits the current viewport to the boundary of the specified map items only.

    \note This method gained the optional \a items argument since Qt 5.15.
    In previous releases, this method fitted the map to all map items.

    \sa fitViewportToVisibleMapItems
*/
void QDeclarativeGeoMap::fitViewportToMapItems(const QVariantList &items)
{
    if (items.size()) {
        QList<QPointer<QDeclarativeGeoMapItemBase> > itms;
        for (const QVariant &i: items) {
            QDeclarativeGeoMapItemBase *itm = qobject_cast<QDeclarativeGeoMapItemBase *>(i.value<QObject *>());
            if (itm)
                itms.append(itm);
        }
        fitViewportToMapItemsRefine(itms, true, false);
    } else {
        fitViewportToMapItemsRefine(m_mapItems, true, false);
    }
}

/*!
    \qmlmethod void QtLocation::Map::fitViewportToVisibleMapItems()

    Fits the current viewport to the boundary of all \b visible map items.
    The camera is positioned in the center of the map items, and at the largest integral
    zoom level possible which allows all map items to be visible on screen.

    \sa fitViewportToMapItems
*/
void QDeclarativeGeoMap::fitViewportToVisibleMapItems()
{
    fitViewportToMapItemsRefine(m_mapItems, true, true);
}

/*!
    \internal
*/
void QDeclarativeGeoMap::fitViewportToMapItemsRefine(const QList<QPointer<QDeclarativeGeoMapItemBase> > &mapItems,
                                                     bool refine,
                                                     bool onlyVisible)
{
    if (!m_map)
        return;

    if (mapItems.size() == 0)
        return;

    double minX = qInf();
    double maxX = -qInf();
    double minY = qInf();
    double maxY = -qInf();
    double topLeftX = 0;
    double topLeftY = 0;
    double bottomRightX = 0;
    double bottomRightY = 0;
    bool haveQuickItem = false;

    // find bounds of all map items
    qsizetype itemCount = 0;
    for (qsizetype i = 0; i < mapItems.count(); ++i) {
        if (!mapItems.at(i))
            continue;
        QDeclarativeGeoMapItemBase *item = mapItems.at(i).data();
        if (!item || (onlyVisible && (!item->isVisible() || item->mapItemOpacity() <= 0.0)))
            continue;

        // skip quick items in the first pass and refine the fit later
        QDeclarativeGeoMapQuickItem *quickItem =
                qobject_cast<QDeclarativeGeoMapQuickItem*>(item);
        if (refine && quickItem) {
                haveQuickItem = true;
                continue;
        }
        // Force map items to update immediately. Needed to ensure correct item size and positions
        // when recursively calling this function.
        // TODO: See if we really need updatePolish on delegated items, in particular
        // in relation to
        // a) fitViewportToMapItems
        // b) presence of MouseArea
        //
        // This is also legacy code. It must be updated to not operate on screen sizes.
        if (item->isPolishScheduled())
           item->updatePolish();

        if (quickItem && quickItem->matrix_ && !quickItem->matrix_->m_matrix.isIdentity()) {
            // TODO: recalculate the center/zoom level so that the item becomes projectable again
            if (quickItem->zoomLevel() == 0.0) // the item is unprojectable, should be skipped.
                continue;

            QRectF brect = item->boundingRect();
            brect = quickItem->matrix_->m_matrix.mapRect(brect);
            QPointF transformedPosition = quickItem->matrix_->m_matrix.map(item->position());
            topLeftX = transformedPosition.x();
            topLeftY = transformedPosition.y();
            bottomRightX = topLeftX + brect.width();
            bottomRightY = topLeftY + brect.height();
        } else {
            QGeoRectangle brect = item->geoShape().boundingGeoRectangle();
            topLeftX = fromCoordinate(brect.topLeft(), false).x();
            topLeftY = fromCoordinate(brect.topLeft(), false).y();
            bottomRightX = fromCoordinate(brect.bottomRight(), false).x();
            bottomRightY = fromCoordinate(brect.bottomRight(), false).y();
        }

        minX = qMin(minX, topLeftX);
        maxX = qMax(maxX, bottomRightX);
        minY = qMin(minY, topLeftY);
        maxY = qMax(maxY, bottomRightY);

        ++itemCount;
    }

    if (itemCount == 0) {
        if (haveQuickItem)
            fitViewportToMapItemsRefine(mapItems, false, onlyVisible);
        return;
    }
    double bboxWidth = maxX - minX;
    double bboxHeight = maxY - minY;
    double bboxCenterX = minX + (bboxWidth / 2.0);
    double bboxCenterY = minY + (bboxHeight / 2.0);

    // position camera to the center of bounding box
    QGeoCoordinate coordinate;
    coordinate = m_map->geoProjection().itemPositionToCoordinate(QDoubleVector2D(bboxCenterX, bboxCenterY), false);
    setProperty("center", QVariant::fromValue(coordinate));

    // adjust zoom
    double bboxWidthRatio = bboxWidth / (bboxWidth + bboxHeight);
    double mapWidthRatio = width() / (width() + height());
    double zoomRatio;

    if (bboxWidthRatio > mapWidthRatio)
        zoomRatio = bboxWidth / width();
    else
        zoomRatio = bboxHeight / height();

    qreal newZoom = std::log10(zoomRatio) / std::log10(0.5);
    newZoom = std::floor(qMax(minimumZoomLevel(), (zoomLevel() + newZoom)));
    setProperty("zoomLevel", QVariant::fromValue(newZoom));

    // as map quick items retain the same screen size after the camera zooms in/out
    // we refine the viewport again to achieve better results
    if (refine)
        fitViewportToMapItemsRefine(mapItems, false, onlyVisible);
}

QT_END_NAMESPACE
