// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtPositioning
import QtLocation
import "../helper.js" as Helper

MapView {
    id: view
    property bool followme: false
    property var scaleLengths: [5, 10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000, 2000000]

    function calculateScale()
    {
        var coord1, coord2, dist, text, f
        f = 0
        coord1 = map.toCoordinate(Qt.point(0,scale.y))
        coord2 = map.toCoordinate(Qt.point(0+scaleImage.sourceSize.width,scale.y))
        dist = Math.round(coord1.distanceTo(coord2))

        if (dist === 0) {
            // not visible
        } else {
            for (var i = 0; i < scaleLengths.length-1; i++) {
                if (dist < (scaleLengths[i] + scaleLengths[i+1]) / 2 ) {
                    f = scaleLengths[i] / dist
                    dist = scaleLengths[i]
                    break;
                }
            }
            if (f === 0) {
                f = dist / scaleLengths[i]
                dist = scaleLengths[i]
            }
        }

        text = Helper.formatDistance(dist)
        scaleImage.width = (scaleImage.sourceSize.width * f) - 2 * scaleImageLeft.sourceSize.width
        scaleText.text = text
    }

    map.center {
        // The Qt Company in Oslo
        latitude: 59.9485
        longitude: 10.7686
    }

    map.onCopyrightLinkActivated: Qt.openUrlExternally(link)

    map.onCenterChanged: {
        scaleTimer.restart()
        if (map.followme)
            if (map.center !== positionSource.position.coordinate) map.followme = false
    }

    map.onZoomLevelChanged: {
        scaleTimer.restart()
        if (map.followme) map.center = positionSource.position.coordinate
    }

    onWidthChanged: {
        scaleTimer.restart()
    }

    onHeightChanged: {
        scaleTimer.restart()
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Plus) {
            map.zoomLevel++
        } else if (event.key === Qt.Key_Minus) {
            map.zoomLevel--
        }
    }

    Timer {
        id: scaleTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: {
            view.calculateScale()
        }
    }

    Item {
        id: scale
        visible: scaleText.text != "0 m"
        z: map.z + 3
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 20
        height: scaleText.height * 2
        width: scaleImage.width

        Image {
            id: scaleImageLeft
            source: Qt.resolvedUrl("../resources/scale_end.png")
            anchors.bottom: parent.bottom
            anchors.right: scaleImage.left
        }
        Image {
            id: scaleImage
            source: Qt.resolvedUrl("../resources/scale.png")
            anchors.bottom: parent.bottom
            anchors.right: scaleImageRight.left
        }
        Image {
            id: scaleImageRight
            source: Qt.resolvedUrl("../resources/scale_end.png")
            anchors.bottom: parent.bottom
            anchors.right: parent.right
        }
        Label {
            id: scaleText
            color: "#004EAE"
            anchors.centerIn: parent
            text: "0 m"
        }
        Component.onCompleted: {
            view.calculateScale();
        }
    }

    MapQuickItem {
        parent: view.map
        id: poiTheQtCompany
        sourceItem: Rectangle { width: 14; height: 14; color: "#e41e25"; border.width: 2; border.color: "white"; smooth: true; radius: 7 }
        coordinate {
            latitude: 59.9485
            longitude: 10.7686
        }
        opacity:1.0
        anchorPoint: Qt.point(sourceItem.width/2, sourceItem.height/2)
    }

    MapQuickItem {
        parent: view.map
        sourceItem: Text{
            text: "The Qt Company"
            color:"#242424"
            font.bold: true
            styleColor: "#ECECEC"
            style: Text.Outline
        }
        coordinate: poiTheQtCompany.coordinate
        anchorPoint: Qt.point(-poiTheQtCompany.sourceItem.width * 0.5,poiTheQtCompany.sourceItem.height * 1.5)
    }

    PositionSource{
        id: positionSource
        active: followme

        onPositionChanged: {
            view.map.center = positionSource.position.coordinate
        }
    }

    Slider {
        id: zoomSlider;
        z: map.z + 3
        from: map.minimumZoomLevel;
        to: map.maximumZoomLevel;
        anchors.margins: 10
        anchors.bottom: scale.top
        anchors.top: parent.top
        anchors.right: parent.right
        orientation : Qt.Vertical
        value: map.zoomLevel
        onValueChanged: {
            map.zoomLevel = value
        }
    }
}
