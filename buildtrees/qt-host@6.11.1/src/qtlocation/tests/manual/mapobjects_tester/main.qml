// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtQuick.Controls as C2
import QtPositioning
import QtLocation

Window {
    id: win
    objectName: "win"
    visible: true
    width: 1440
    height: 720
    title: qsTr("MapObjects tester")


    property real initialZL: 5
    property var initialCenter:  QtPositioning.coordinate(66.9961, -175.012)
    property real rotation : 0

    Shortcut {
        sequence: "Ctrl+R"
        onActivated: {
            rotation += 30
        }
    }

    function syncMaps(mFrom, mTo, propName)
    {
        if (mTo[propName] !== mFrom[propName]) {
            mTo[propName] = mFrom[propName]
        }
    }

    Plugin {
        id: osm // use only one plugin, to avoid messing up the cache
        name: "osm"
    }
    Rectangle {
        id: mapContainer
        rotation: win.rotation
        anchors.fill: parent
        color: "lightsteelblue"

        Map {
            id: map
            rotation: win.rotation
            objectName: "map"
            anchors {
                bottom: parent.bottom
                top: parent.top
                left: parent.horizontalCenter
                right: parent.right
            }

            color: 'transparent'
            plugin: osm
            activeMapType: map.supportedMapTypes[2]
            center: initialCenter
            zoomLevel: initialZL
            copyrightsVisible: false

            Component.onCompleted: {
                var o = migComponent.createObject(map)
                o.glPolygons  = Qt.binding(function() {return switchPolygons2.checked})
                o.glPolylines  = Qt.binding(function() {return switchPolylines2.checked})
                o.glCircles  = Qt.binding(function() {return switchCircles2.checked})
                o.glRectangles  = Qt.binding(function() {return switchRectangles2.checked})
                map.addMapItemGroup(o);
            }

            C2.Switch {
                text: qsTr("OGL Polygons")
                id: switchPolygons2
                anchors {
                    top: parent.top
                    right: parent.right
                    topMargin: 12
                    rightMargin: 12
                }
            }
            C2.Switch {
                text: qsTr("OGL Polylines")
                id: switchPolylines2
                anchors {
                    top: switchPolygons2.bottom
                    right: parent.right
                    topMargin: 12
                    rightMargin: 12
                }
            }
            C2.Switch {
                text: qsTr("OGL Circles")
                id: switchCircles2
                anchors {
                    top: switchPolylines2.bottom
                    right: parent.right
                    topMargin: 12
                    rightMargin: 12
                }
            }
            C2.Switch {
                text: qsTr("OGL Rectangles")
                id: switchRectangles2
                anchors {
                    top: switchCircles2.bottom
                    right: parent.right
                    topMargin: 12
                    rightMargin: 12
                }
            }
        }
    }

    Component {
        id: migComponent
        MapItemGroup {
            id: polyGroup
            property bool glPolygons
            property bool glPolylines
            property bool glCircles
            property bool glRectangles
            objectName: parent.objectName + "_MIG_"

            MapPolyline {
                id: tstPolyLine // to verify the polygon stays where it's supposed to
                line.color: 'black'
                objectName: parent.objectName + "black"
                line.width: 1
                opacity: 1.0
                path: [
                    { latitude: 76.9965, longitude: -175.012 },
                    { latitude: 26.9965, longitude: -175.012 }
                ]
            }

            MapPolyline {
                id: timeline
                line.color: "red"
                objectName: parent.objectName + "timeline"
                line.width: 4
                path: [
                    { latitude: 90, longitude: 180 },
                    { latitude: -90, longitude: -180 }
                ]
            }

            MapPolygon {
                id: poly1
                color: "red"
                objectName: parent.objectName + "red"
                path: [
                    { latitude: 55, longitude: 170 },
                    { latitude: 66.9965, longitude: -175.012 },
                    { latitude: 55, longitude: -160 },
                    { latitude: 40, longitude: -165 },
                    { latitude: 45, longitude: 178 }
                ]
                MouseArea {
                    anchors.fill: parent
                    onClicked: console.log("poly1 in "+parent.parent.objectName + "clicked")

                    Rectangle { // this is technically unsupported, but practically works.
                        color: "transparent"
                        border.color: "red"
                        anchors.fill: parent
                    }
                }
            }

            MapPolygon {
                id: selfIntersectingPolygon
                color: 'darkmagenta'
                objectName: parent.objectName + "darkmagenta"
                opacity: 1.0
                path: [
                    { latitude: 19, longitude: 49 },
                    { latitude: 18, longitude: 49 },
                    { latitude: 18, longitude: 51 },
                    { latitude: 20, longitude: 51 },
                    { latitude: 20, longitude: 50 },
                    { latitude: 18.5, longitude: 50 },
                    { latitude: 18.5, longitude: 52 },
                    { latitude: 19, longitude: 52 }
                ]

                MouseArea{
                    anchors.fill: parent
                    drag.target: parent // This one might glitch as the tessellation done by earcut might be
                                        // inaccurate, so clicking in an area that is covered by earcut but
                                        // does not exist will not trigger the panning

                    Rectangle { // this is technically unsupported, but practically works.
                        color: "transparent"
                        border.color: "red"
                        anchors.fill: parent
                    }
                }
            }

            MapPolygon {
                id: poly2
                color: "green"
                border.color: "black"
                border.width: 8
                objectName: parent.objectName + "green"
                path: [
                    { latitude: -45, longitude: -170 },
                    { latitude: -55, longitude: -155 },
                    { latitude: -45, longitude: -130 },
                    { latitude: -35, longitude: -155 }
                ]
                MouseArea{
                    anchors.fill: parent
                    drag.target: parent
                    Rectangle { // this is technically unsupported, but practically works.
                        color: "transparent"
                        border.color: "red"
                        anchors.fill: parent
                    }
                }
            }

            MapPolygon {
                id: poly3
                color: "deepskyblue"
                objectName: parent.objectName + "deepskyblue"
                opacity: 0.2
                path: [
                    { latitude: 65, longitude: -20 },
                    { latitude: 75, longitude: 140 },
                    { latitude: 65, longitude: 80 },
                    { latitude: 55, longitude: -30 }
                ]
            }

            MapRectangle {
                id: rect
                color: 'tomato'
                border.color: 'black'
                border.width: 6
                topLeft: QtPositioning.coordinate(10,-10)
                bottomRight: QtPositioning.coordinate(-10,10)
                MouseArea {
                    anchors.fill: parent
                    Rectangle {
                        color: "transparent"
                        border.color: "red"
                        border.width: 1
                        anchors.fill: parent
                    }
                }
            }

            LongPolyline {
                id: longPoly
                Component.onCompleted: longPolyPath = path
            }

            MapCircle {
                center: QtPositioning.coordinate(52, 0)
                radius: sliRadius.value
                color: 'deepskyblue'
                border.width: 6
                border.color: 'firebrick'
                MouseArea {
                    anchors.fill: parent
                    Rectangle {
                        color: "transparent"
                        border.color: "red"
                        border.width: 1
                        anchors.fill: parent
                    }
                }
            }

            MapCircle {
                id: circle1
                border.color: 'deepskyblue'
                border.width: 26
                center: QtPositioning.coordinate(17, 44);
                radius: 200*1000
                color: "firebrick"
            }
        }
    }

    property var longPolyPath

    C2.Slider {
        id: sliRadius
        orientation: Qt.Vertical
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            topMargin: 10
            leftMargin: 10
            bottomMargin: 10
        }
        from: 30 * 1000
        to: 600 * 1000
        value: 100 * 1000
    }
}
