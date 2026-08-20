// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.13 as C2
import QtPositioning 5.6
import QtLocation 5.15

Window {
    id: win
    visible: true
    width: 1600
    height: 800
    title: qsTr("MapPolyline tester")

    property real initialZL:  4
    property var initialCenter:  QtPositioning.coordinate(55, -160)
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
            objectName: "map1"
            anchors {
                bottom: parent.bottom
                top: parent.top
                left: parent.left
                right: (leftSwitch.checked) ? parent.right : parent.horizontalCenter
            }

            onCenterChanged:        syncMaps(map, map2, "center")
            onTiltChanged:          syncMaps(map, map2, "tilt")
            onBearingChanged:       syncMaps(map, map2, "bearing")
            onZoomLevelChanged:     syncMaps(map, map2, "zoomLevel")
            onFieldOfViewChanged:   syncMaps(map, map2, "fieldOfView")

            opacity: 1.0
            color: 'transparent'
            plugin: osm
            center: initialCenter
            activeMapType: map.supportedMapTypes[2]
            zoomLevel: initialZL
            z : parent.z + 1
            copyrightsVisible: false

            Component.onCompleted: {
                var o = migComponent.createObject(map)
                o.glPolylines  = Qt.binding(function() {return switchPolylines1.currentText})
                map.addMapItemGroup(o);
            }
            MouseArea {
                anchors.fill: parent
                onClicked: (mouse) => {
                    mouse.accepted = false
                    var crd = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                    var s = crd.toString(0)
                    console.log("Clicked the map (not an Item) on ",s)
                }
            }

            C2.ComboBox {
                model: ['Software', 'OpenGL']
                id: switchPolylines1
                anchors {
                    top: parent.top
                    right: parent.right
                    topMargin: 12
                    rightMargin: 12
                }
            }
        }
        Map {
            id: map2
            rotation: win.rotation
            objectName: "map2"
            visible: !leftSwitch.checked
            anchors {
                bottom: parent.bottom
                top: parent.top
                left: parent.horizontalCenter
                right: parent.right
            }

            onCenterChanged:        syncMaps(map2, map, "center")
            onTiltChanged:          syncMaps(map2, map, "tilt")
            onBearingChanged:       syncMaps(map2, map, "bearing")
            onZoomLevelChanged:     syncMaps(map2, map, "zoomLevel")
            onFieldOfViewChanged:   syncMaps(map2, map, "fieldOfView")

            color: 'transparent'
            plugin: osm
            activeMapType: map.supportedMapTypes[2]
            center: initialCenter
            zoomLevel: initialZL
            copyrightsVisible: false

            Component.onCompleted: {
                var o = migComponent.createObject(map2)
                o.glPolylines  = Qt.binding(function() {return switchPolylines2.currentText})
                map2.addMapItemGroup(o);
            }

            C2.ComboBox {
                model: ['Software', 'OpenGL']
                id: switchPolylines2
                anchors {
                    top: parent.top
                    right: parent.right
                    topMargin: 12
                    rightMargin: 12
                }
                onCurrentTextChanged: console.log("CURRENT TEXT CHANGED ",currentText)
            }
        }
    }

    Component {
        id: migComponent
        MapItemGroup {
            id: polyGroup
            property bool glPolygons : true
            property string glPolylines : "Software"
            property bool glCircles : true
            property bool glRectangles : true
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

            MapPolyline {
                id: poly1
                line.color: "pink"
                line.width: sliWidth.value
                objectName: parent.objectName + "red"

                path: [
                    { latitude: 55, longitude: 170 },
                    { latitude: 66.9965, longitude: 170 },
                    { latitude: 66.9965, longitude: -175.012 },
                    { latitude: 55, longitude: -160 },
                    { latitude: 40, longitude: -165 },
                    { latitude: 45, longitude: 174 },
                    { latitude: 43, longitude: -168 }
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
                id: poly2
                color: "green"
                border.color: "black"
                border.width: 12
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
        }
    }


    C2.Switch {
        text: qsTr("Left")
        id: leftSwitch
        anchors {
            top: parent.top
            left: parent.left
            leftMargin: 12
            rightMargin: 12
        }
        checked: false
    }
    C2.Slider {
        id: sliWidth
        orientation: Qt.Vertical
        anchors {
            left: parent.left
            top: leftSwitch.bottom
            bottom: parent.bottom
            topMargin: 10
            leftMargin: 10
            bottomMargin: 10
        }
        from: 1
        to: 50
        value: 50
    }
}
