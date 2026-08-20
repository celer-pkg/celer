// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qdeclarativegeomapitemgroup_p.h"
#include "qdeclarativegeomapitembase_p.h"
#include "qdeclarativegeomap_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype MapItemGroup
    \nativetype QDeclarativeGeoMapItemGroup
    \inqmlmodule QtLocation
    \ingroup qml-QtLocation5-maps
    \since QtLocation 5.9

    \brief The MapItemGroup type is a container for map items.

    Its purpose is to enable code modularization by allowing the usage
    of qml files containing map elements related to each other, and
    the associated bindings.

    \note The release of this API with Qt 5.9 is a Technology Preview.

    \section2 Example Usage

    The following snippet shows how to use a MapItemGroup to create a MapCircle, centered at
    the coordinate (63, -18) with a radius of 100km, filled in red, surrounded by an ondulated green border,
    both contained in a semitransparent blue circle with a MouseArea that moves the whole group.
    This group is defined in a separate file named PolygonGroup.qml:

    \code
    import QtQuick
    import QtPositioning
    import QtLocation

    MapItemGroup {
        id: itemGroup
        property alias position: mainCircle.center
        property var radius: 100 * 1000
        property var borderHeightPct : 0.3

        MapCircle {
            id: mainCircle
            center : QtPositioning.coordinate(40, 0)
            radius: itemGroup.radius * (1.0 + borderHeightPct)
            opacity: 0.05
            visible: true
            color: 'blue'

            MouseArea{
                anchors.fill: parent
                drag.target: parent
                id: maItemGroup
            }
        }

        MapCircle {
            id: groupCircle
            center: itemGroup.position
            radius: itemGroup.radius
            color: 'crimson'

            onCenterChanged: {
                groupPolyline.populateBorder();
            }
        }

        MapPolyline {
            id: groupPolyline
            line.color: 'green'
            line.width: 3

            function populateBorder() {
                groupPolyline.path = [] // clearing the path
                var waveLength = 8.0;
                var waveAmplitude = groupCircle.radius * borderHeightPct;
                for (var i=0; i <= 360; i++) {
                    var wavePhase = (i/360.0 * 2.0 * Math.PI )* waveLength
                    var waveHeight = (Math.cos(wavePhase) + 1.0) / 2.0
                    groupPolyline.addCoordinate(groupCircle.center.atDistanceAndAzimuth(groupCircle.radius + waveAmplitude * waveHeight , i))
                }
            }

            Component.onCompleted: {
                populateBorder()
            }
        }
    }
    \endcode

    PolygonGroup.qml is now a reusable component that can then be used in a Map as:

    \code
    Map {
        id: map
        PolygonGroup {
            id: polygonGroup
            position: QtPositioning.coordinate(63,-18)
        }
    }
    \endcode

    \image api-mapitemgroup.png
*/

QDeclarativeGeoMapItemGroup::QDeclarativeGeoMapItemGroup(QQuickItem *parent)
    : QQuickItem(parent)
{
    connect(this, &QQuickItem::opacityChanged,
            this, &QDeclarativeGeoMapItemGroup::mapItemOpacityChanged);
}

QDeclarativeGeoMapItemGroup::~QDeclarativeGeoMapItemGroup()
{

}

void QDeclarativeGeoMapItemGroup::setParentGroup(QDeclarativeGeoMapItemGroup &parentGroup)
{
    m_parentGroup = &parentGroup;
    connect(m_parentGroup, &QDeclarativeGeoMapItemGroup::mapItemOpacityChanged,
            this, &QDeclarativeGeoMapItemGroup::mapItemOpacityChanged);
}

void QDeclarativeGeoMapItemGroup::setQuickMap(QDeclarativeGeoMap *quickMap)
{
    if (!quickMap && m_quickMap)
        m_quickMap->disconnect(this);
    m_quickMap = quickMap;
    if (m_quickMap) {
        onMapSizeChanged();
        connect(m_quickMap, &QQuickItem::widthChanged, this, &QDeclarativeGeoMapItemGroup::onMapSizeChanged);
        connect(m_quickMap, &QQuickItem::heightChanged, this, &QDeclarativeGeoMapItemGroup::onMapSizeChanged);
    }
}

QDeclarativeGeoMap *QDeclarativeGeoMapItemGroup::quickMap() const
{
    return m_quickMap;
}

qreal QDeclarativeGeoMapItemGroup::mapItemOpacity() const
{
    return ((m_parentGroup) ? m_parentGroup->mapItemOpacity() : 1.0) * opacity();
}

void QDeclarativeGeoMapItemGroup::classBegin()
{
    QQuickItem::classBegin();
}

void QDeclarativeGeoMapItemGroup::componentComplete()
{
    QQuickItem::componentComplete();

    // In certain cases the parent won't be set via the constructor, but rather later on
    // during the instantiation/incubation process.
    // Therefore calling setParentGroup here, when the parent is known.
    // The childrenChanged use case to handle dynamically-added items is currently unsupported.
    const QList<QQuickItem *> &quickKids = childItems();
    for (QQuickItem *k : quickKids) {
        QDeclarativeGeoMapItemGroup *childGroup
                = qobject_cast<QDeclarativeGeoMapItemGroup *>(k);
        if (childGroup) {
            childGroup->setParentGroup(*this);
            continue;
        }
        QDeclarativeGeoMapItemBase *childItem
                = qobject_cast<QDeclarativeGeoMapItemBase *>(k);
        if (childItem) {
            childItem->setParentGroup(*this);
            continue;
        }
    }
}

void QDeclarativeGeoMapItemGroup::onMapSizeChanged()
{
    setWidth(m_quickMap->width());
    setHeight(m_quickMap->height());
}

QT_END_NAMESPACE
