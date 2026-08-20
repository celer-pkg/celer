// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtLocation
import QtPositioning

Item {
    id: mapView
    property alias lightningLayerVisible: lightningLayer.visible
    property alias distanceLayerVisible: distanceLayer.visible
    property alias locationAllowed: positionSource.active
    property alias zoomLevel: map.zoomLevel
    property int mapType: MapTypeConfig.map

    function recenter(coordinate) {
        map.center = coordinate ?? positionSource.position.coordinate;
    }

    PositionSource {
        id: positionSource
        updateInterval: 3000
        active: false
        preferredPositioningMethods: PositionSource.AllPositioningMethods
        onPositionChanged: LightningController.setUserLocation(position.coordinate)
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: Plugin {
            name: "osm"
            PluginParameter {
                name: "osm.mapping.providersrepository.disabled"
                value: "true"
            }
        }

        MapQuickItem {
            coordinate: positionSource.position.coordinate
            sourceItem: Image {
                id: userLocationImage
                source: "icons/user-location.svg"
                width: 24
                height: width
                anchors.centerIn: parent
            }
        }

        LightningMapLayer { id: lightningLayer }

        DistanceTimeLayer {
            id: distanceLayer
            anchors.top: map.top
            anchors.horizontalCenter: map.horizontalCenter
            anchors.margins: 30
            width: 180
            height: 90
        }
    }

    PinchHandler {
        target: null
        grabPermissions: PointerHandler.TakeOverForbidden
        property geoCoordinate startCenteroid
        onActiveChanged: {
            if (active)
                startCenteroid = map.toCoordinate(centroid.position, false)
        }
        onScaleChanged: (delta) => {
            map.zoomLevel += Math.log(delta)
            map.alignCoordinateToPoint(startCenteroid, centroid.position)
        }
    }

    WheelHandler {
        onWheel: function(event) {
            const loc = map.toCoordinate(point.position)
            map.zoomLevel += event.angleDelta.y / 120;
            map.alignCoordinateToPoint(loc, point.position)
        }
    }

    DragHandler {
        target: null
        grabPermissions: PointerHandler.TakeOverForbidden
        onTranslationChanged: (delta) => { map.pan(-delta.x, -delta.y); }
    }

    onMapTypeChanged: {
        let index = 0;
        for (; index < map.supportedMapTypes.length; ++index) {
            const supportedMapType = map.supportedMapTypes[index];
            if (supportedMapType.style === mapType)
                break;
        }
        if (index >= map.supportedMapTypes.length) {
            return;
        }
        map.activeMapType = map.supportedMapTypes[index];
    }
}
