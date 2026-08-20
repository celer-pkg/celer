// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Map {
    width: 1024
    height: 1024

    property double lonPos: 30
    center: QtPositioning.coordinate(0, lonPos)

    NumberAnimation on lonPos
    {
        loops: Animation.Infinite
        from: -180
        to: 180
        duration: 30000
    }

    id: map
    plugin: Plugin {
        name: "osm"
    }
    zoomLevel: 1
    copyrightsVisible: false

    Repeater {
        id: rectangles
        property var colors: [
            "red",
            "orange",
            "yellow",
            "green",
            "blue",
            "violet"
        ]
        property int gridSize: rectangles.colors.length*4
        model: gridSize*gridSize
        MapRectangle
        {
            property double center_longitude: -180+360*(index%rectangles.gridSize+0.5)/(rectangles.gridSize)
            property double center_latitude: -90+180*(Math.floor(index/rectangles.gridSize)+0.5)/(rectangles.gridSize)
            property double a: 1

            topLeft
            {
                longitude: Math.min(180, Math.max(-180, center_longitude - a/2))
                latitude: Math.min(90, Math.max(-90, center_latitude - a/2))
            }
            bottomRight
            {
                longitude: Math.min(180, Math.max(-180, center_longitude + a/2))
                latitude: Math.min(90, Math.max(-90, center_latitude + a/2))
            }
            color: rectangles.colors[Math.floor(index%rectangles.colors.length)]
            border.width: 2
            autoFadeIn: false
            opacity: 0.1
            NumberAnimation on a
            {
                loops: Animation.Infinite
                from: 1
                to: 80
                duration: 10000
            }
        }
    }

    Keys.onPressed: (event)=> {
        Qt.quit()
    }
}
