// Copyright (C) 2017 Mapbox, Inc.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qmapboxglstylechange_p.h"

#include <QtCore/QDebug>
#include <QtCore/QMetaProperty>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>
#include <QtPositioning/QGeoPath>
#include <QtPositioning/QGeoPolygon>
#include <QtQml/QJSValue>
#include <QtLocation/private/qdeclarativecirclemapitem_p_p.h>

namespace {

QByteArray formatPropertyName(const QByteArray &name)
{
    QString nameAsString = QString::fromLatin1(name);
    static const QRegularExpression camelCaseRegex(QStringLiteral("([a-z0-9])([A-Z])"));
    return nameAsString.replace(camelCaseRegex, QStringLiteral("\\1-\\2")).toLower().toLatin1();
}

bool isImmutableProperty(const QByteArray &name)
{
    return name == QStringLiteral("type") || name == QStringLiteral("layer");
}

QString getId(QDeclarativeGeoMapItemBase *mapItem)
{
    return QStringLiteral("QtLocation-") +
            ((mapItem->objectName().isEmpty()) ? QString::number(quint64(mapItem)) : mapItem->objectName());
}

// Mapbox GL supports geometry segments that spans above 180 degrees in
// longitude. To keep visual expectations in parity with Qt, we need to adapt
// the coordinates to always use the shortest path when in ambiguity.
static bool geoRectangleCrossesDateLine(const QGeoRectangle &rect) {
    return rect.topLeft().longitude() > rect.bottomRight().longitude();
}

QMapbox::Feature featureFromMapRectangle(QDeclarativeRectangleMapItem *mapItem)
{
    const QGeoRectangle *rect = static_cast<const QGeoRectangle *>(&mapItem->geoShape());
    QMapbox::Coordinate bottomLeft { rect->bottomLeft().latitude(), rect->bottomLeft().longitude() };
    QMapbox::Coordinate topLeft { rect->topLeft().latitude(), rect->topLeft().longitude() };
    QMapbox::Coordinate bottomRight { rect->bottomRight().latitude(), rect->bottomRight().longitude() };
    QMapbox::Coordinate topRight { rect->topRight().latitude(), rect->topRight().longitude() };
    if (geoRectangleCrossesDateLine(*rect)) {
        bottomRight.second += 360.0;
        topRight.second += 360.0;
    }
    QMapbox::CoordinatesCollections geometry { { { bottomLeft, bottomRight, topRight, topLeft, bottomLeft } } };

    return QMapbox::Feature(QMapbox::Feature::PolygonType, geometry, {}, getId(mapItem));
}

QMapbox::Feature featureFromMapCircle(QDeclarativeCircleMapItem *mapItem)
{
    static const int circleSamples = 128;
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(mapItem->map()->geoProjection());
    QList<QGeoCoordinate> path;
    QGeoCoordinate leftBound;
    QDeclarativeCircleMapItemPrivate::calculatePeripheralPoints(path, mapItem->center(), mapItem->radius(), circleSamples, leftBound);
    QList<QDoubleVector2D> pathProjected;
    for (const QGeoCoordinate &c : std::as_const(path))
        pathProjected << p.geoToMapProjection(c);
    if (QDeclarativeCircleMapItemPrivateCPU::crossEarthPole(mapItem->center(), mapItem->radius()))
        QDeclarativeCircleMapItemPrivateCPU::preserveCircleGeometry(pathProjected, mapItem->center(), mapItem->radius(), p);
    path.clear();
    for (const QDoubleVector2D &c : std::as_const(pathProjected))
        path << p.mapProjectionToGeo(c);


    QMapbox::Coordinates coordinates;
    for (const QGeoCoordinate &coordinate : path) {
        coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() };
    }
    coordinates.append(coordinates.first());  // closing the path
    QMapbox::CoordinatesCollections geometry { { coordinates } };
    return QMapbox::Feature(QMapbox::Feature::PolygonType, geometry, {}, getId(mapItem));
}

static QMapbox::Coordinates qgeocoordinate2mapboxcoordinate(const QList<QGeoCoordinate> &crds, const bool crossesDateline, bool closed = false)
{
    QMapbox::Coordinates coordinates;
    for (const QGeoCoordinate &coordinate : crds) {
        if (!coordinates.empty() && crossesDateline && qAbs(coordinate.longitude() - coordinates.last().second) > 180.0) {
            coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() + (coordinate.longitude() >= 0 ? -360.0 : 360.0) };
        } else {
            coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() };
        }
    }
    if (closed && !coordinates.empty() && coordinates.last() != coordinates.first())
        coordinates.append(coordinates.first());  // closing the path
    return coordinates;
}

QMapbox::Feature featureFromMapPolygon(QDeclarativePolygonMapItem *mapItem)
{
    const QGeoPolygon *polygon = static_cast<const QGeoPolygon *>(&mapItem->geoShape());
    const bool crossesDateline = geoRectangleCrossesDateLine(polygon->boundingGeoRectangle());
    QMapbox::CoordinatesCollections geometry;
    QMapbox::CoordinatesCollection poly;
    QMapbox::Coordinates coordinates = qgeocoordinate2mapboxcoordinate(polygon->perimeter(), crossesDateline, true);
    poly.push_back(coordinates);
    for (int i = 0; i < polygon->holesCount(); ++i) {
        coordinates = qgeocoordinate2mapboxcoordinate(polygon->holePath(i), crossesDateline, true);
        poly.push_back(coordinates);
    }

    geometry.push_back(poly);
    return QMapbox::Feature(QMapbox::Feature::PolygonType, geometry, {}, getId(mapItem));
}

QMapbox::Feature featureFromMapPolyline(QDeclarativePolylineMapItem *mapItem)
{
    const QGeoPath *path = static_cast<const QGeoPath *>(&mapItem->geoShape());
    QMapbox::Coordinates coordinates;
    const bool crossesDateline = geoRectangleCrossesDateLine(path->boundingGeoRectangle());
    for (const QGeoCoordinate &coordinate : path->path()) {
        if (!coordinates.empty() && crossesDateline && qAbs(coordinate.longitude() - coordinates.last().second) > 180.0) {
            coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() + (coordinate.longitude() >= 0 ? -360.0 : 360.0) };
        } else {
            coordinates << QMapbox::Coordinate { coordinate.latitude(), coordinate.longitude() };
        }
    }
    QMapbox::CoordinatesCollections geometry { { coordinates } };

    return QMapbox::Feature(QMapbox::Feature::LineStringType, geometry, {}, getId(mapItem));
}

QMapbox::Feature featureFromMapItem(QDeclarativeGeoMapItemBase *item)
{
    switch (item->itemType()) {
    case QGeoMap::MapRectangle:
        return featureFromMapRectangle(static_cast<QDeclarativeRectangleMapItem *>(item));
    case QGeoMap::MapCircle:
        return featureFromMapCircle(static_cast<QDeclarativeCircleMapItem *>(item));
    case QGeoMap::MapPolygon:
        return featureFromMapPolygon(static_cast<QDeclarativePolygonMapItem *>(item));
    case QGeoMap::MapPolyline:
        return featureFromMapPolyline(static_cast<QDeclarativePolylineMapItem *>(item));
    default:
        qWarning() << "Unsupported QGeoMap item type: " << item->itemType();
        return QMapbox::Feature();
    }
}

QList<QByteArray> getAllPropertyNamesList(QObject *object)
{
    const QMetaObject *metaObject = object->metaObject();
    QList<QByteArray> propertyNames(object->dynamicPropertyNames());
    for (int i = metaObject->propertyOffset(); i < metaObject->propertyCount(); ++i) {
        propertyNames.append(metaObject->property(i).name());
    }
    return propertyNames;
}

} // namespace


QList<QSharedPointer<QMapboxGLStyleChange>> QMapboxGLStyleChange::addMapItem(QDeclarativeGeoMapItemBase *item, const QString &before)
{
    QList<QSharedPointer<QMapboxGLStyleChange>> changes;

    switch (item->itemType()) {
    case QGeoMap::MapRectangle:
    case QGeoMap::MapCircle:
    case QGeoMap::MapPolygon:
    case QGeoMap::MapPolyline:
        break;
    default:
        qWarning() << "Unsupported QGeoMap item type: " << item->itemType();
        return changes;
    }

    QMapbox::Feature feature = featureFromMapItem(item);

    changes << QMapboxGLStyleAddLayer::fromFeature(feature, before);
    changes << QMapboxGLStyleAddSource::fromFeature(feature);
    changes << QMapboxGLStyleSetPaintProperty::fromMapItem(item);
    changes << QMapboxGLStyleSetLayoutProperty::fromMapItem(item);

    return changes;
}

QList<QSharedPointer<QMapboxGLStyleChange>> QMapboxGLStyleChange::removeMapItem(QDeclarativeGeoMapItemBase *item)
{
    QList<QSharedPointer<QMapboxGLStyleChange>> changes;

    const QString id = getId(item);

    changes << QSharedPointer<QMapboxGLStyleChange>(new QMapboxGLStyleRemoveLayer(id));
    changes << QSharedPointer<QMapboxGLStyleChange>(new QMapboxGLStyleRemoveSource(id));

    return changes;
}

// QMapboxGLStyleSetLayoutProperty

void QMapboxGLStyleSetLayoutProperty::apply(QMapboxGL *map)
{
    map->setLayoutProperty(m_layer, m_property, m_value);
}

QList<QSharedPointer<QMapboxGLStyleChange>> QMapboxGLStyleSetLayoutProperty::fromMapItem(QDeclarativeGeoMapItemBase *item)
{
    QList<QSharedPointer<QMapboxGLStyleChange>> changes;

    switch (item->itemType()) {
    case QGeoMap::MapPolyline:
        changes = fromMapItem(static_cast<QDeclarativePolylineMapItem *>(item));
    default:
        break;
    }

    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetLayoutProperty(getId(item), QStringLiteral("visibility"),
            item->isVisible() ? QStringLiteral("visible") : QStringLiteral("none")));

    return changes;
}

QList<QSharedPointer<QMapboxGLStyleChange>> QMapboxGLStyleSetLayoutProperty::fromMapItem(QDeclarativePolylineMapItem *item)
{
    QList<QSharedPointer<QMapboxGLStyleChange>> changes;
    changes.reserve(2);

    const QString id = getId(item);

    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetLayoutProperty(id, QStringLiteral("line-cap"), QStringLiteral("square")));
    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetLayoutProperty(id, QStringLiteral("line-join"), QStringLiteral("bevel")));

    return changes;
}

QMapboxGLStyleSetLayoutProperty::QMapboxGLStyleSetLayoutProperty(const QString& layer, const QString& property, const QVariant &value)
    : m_layer(layer), m_property(property), m_value(value)
{
}

// QMapboxGLStyleSetPaintProperty

QMapboxGLStyleSetPaintProperty::QMapboxGLStyleSetPaintProperty(const QString& layer, const QString& property, const QVariant &value)
    : m_layer(layer), m_property(property), m_value(value)
{
}

void QMapboxGLStyleSetPaintProperty::apply(QMapboxGL *map)
{
    map->setPaintProperty(m_layer, m_property, m_value);
}

QList<QSharedPointer<QMapboxGLStyleChange>> QMapboxGLStyleSetPaintProperty::fromMapItem(QDeclarativeGeoMapItemBase *item)
{
    switch (item->itemType()) {
    case QGeoMap::MapRectangle:
        return fromMapItem(static_cast<QDeclarativeRectangleMapItem *>(item));
    case QGeoMap::MapCircle:
        return fromMapItem(static_cast<QDeclarativeCircleMapItem *>(item));
    case QGeoMap::MapPolygon:
        return fromMapItem(static_cast<QDeclarativePolygonMapItem *>(item));
    case QGeoMap::MapPolyline:
        return fromMapItem(static_cast<QDeclarativePolylineMapItem *>(item));
    default:
        qWarning() << "Unsupported QGeoMap item type: " << item->itemType();
        return QList<QSharedPointer<QMapboxGLStyleChange>>();
    }
}

QList<QSharedPointer<QMapboxGLStyleChange>> QMapboxGLStyleSetPaintProperty::fromMapItem(QDeclarativeRectangleMapItem *item)
{
    QList<QSharedPointer<QMapboxGLStyleChange>> changes;
    changes.reserve(3);

    const QString id = getId(item);

    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("fill-opacity"), item->color().alphaF() * item->mapItemOpacity()));
    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("fill-color"), item->color()));
    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("fill-outline-color"), item->border()->color()));

    return changes;
}

QList<QSharedPointer<QMapboxGLStyleChange>> QMapboxGLStyleSetPaintProperty::fromMapItem(QDeclarativeCircleMapItem *item)
{
    QList<QSharedPointer<QMapboxGLStyleChange>> changes;
    changes.reserve(3);

    const QString id = getId(item);

    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("fill-opacity"), item->color().alphaF() * item->mapItemOpacity()));
    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("fill-color"), item->color()));
    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("fill-outline-color"), item->border()->color()));

    return changes;
}

QList<QSharedPointer<QMapboxGLStyleChange>> QMapboxGLStyleSetPaintProperty::fromMapItem(QDeclarativePolygonMapItem *item)
{
    QList<QSharedPointer<QMapboxGLStyleChange>> changes;
    changes.reserve(3);

    const QString id = getId(item);

    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("fill-opacity"), item->color().alphaF() * item->mapItemOpacity()));
    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("fill-color"), item->color()));
    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("fill-outline-color"), item->border()->color()));

    return changes;
}

QList<QSharedPointer<QMapboxGLStyleChange>> QMapboxGLStyleSetPaintProperty::fromMapItem(QDeclarativePolylineMapItem *item)
{
    QList<QSharedPointer<QMapboxGLStyleChange>> changes;
    changes.reserve(3);

    const QString id = getId(item);

    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("line-opacity"), item->line()->color().alphaF() * item->mapItemOpacity()));
    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("line-color"), item->line()->color()));
    changes << QSharedPointer<QMapboxGLStyleChange>(
        new QMapboxGLStyleSetPaintProperty(id, QStringLiteral("line-width"), item->line()->width()));

    return changes;
}

// QMapboxGLStyleAddLayer

void QMapboxGLStyleAddLayer::apply(QMapboxGL *map)
{
    map->addLayer(m_params, m_before);
}

QSharedPointer<QMapboxGLStyleChange> QMapboxGLStyleAddLayer::fromFeature(const QMapbox::Feature &feature, const QString &before)
{
    auto layer = new QMapboxGLStyleAddLayer();
    layer->m_params[QStringLiteral("id")] = feature.id;
    layer->m_params[QStringLiteral("source")] = feature.id;

    switch (feature.type) {
    case QMapbox::Feature::PointType:
        layer->m_params[QStringLiteral("type")] = QStringLiteral("circle");
        break;
    case QMapbox::Feature::LineStringType:
        layer->m_params[QStringLiteral("type")] = QStringLiteral("line");
        break;
    case QMapbox::Feature::PolygonType:
        layer->m_params[QStringLiteral("type")] = QStringLiteral("fill");
        break;
    }

    layer->m_before = before;

    return QSharedPointer<QMapboxGLStyleChange>(layer);
}


// QMapboxGLStyleRemoveLayer

void QMapboxGLStyleRemoveLayer::apply(QMapboxGL *map)
{
    map->removeLayer(m_id);
}

QMapboxGLStyleRemoveLayer::QMapboxGLStyleRemoveLayer(const QString &id) : m_id(id)
{
}


// QMapboxGLStyleAddSource

void QMapboxGLStyleAddSource::apply(QMapboxGL *map)
{
    map->updateSource(m_id, m_params);
}

QSharedPointer<QMapboxGLStyleChange> QMapboxGLStyleAddSource::fromFeature(const QMapbox::Feature &feature)
{
    auto source = new QMapboxGLStyleAddSource();

    source->m_id = feature.id.toString();
    source->m_params[QStringLiteral("type")] = QStringLiteral("geojson");
    source->m_params[QStringLiteral("data")] = QVariant::fromValue<QMapbox::Feature>(feature);

    return QSharedPointer<QMapboxGLStyleChange>(source);
}

QSharedPointer<QMapboxGLStyleChange> QMapboxGLStyleAddSource::fromMapItem(QDeclarativeGeoMapItemBase *item)
{
    return fromFeature(featureFromMapItem(item));
}


// QMapboxGLStyleRemoveSource

void QMapboxGLStyleRemoveSource::apply(QMapboxGL *map)
{
    map->removeSource(m_id);
}

QMapboxGLStyleRemoveSource::QMapboxGLStyleRemoveSource(const QString &id) : m_id(id)
{
}


// QMapboxGLStyleSetFilter

void QMapboxGLStyleSetFilter::apply(QMapboxGL *map)
{
    map->setFilter(m_layer, m_filter);
}

// QMapboxGLStyleAddImage

void QMapboxGLStyleAddImage::apply(QMapboxGL *map)
{
    map->addImage(m_name, m_sprite);
}
