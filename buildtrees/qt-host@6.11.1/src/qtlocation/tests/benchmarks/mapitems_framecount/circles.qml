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
        id: circles
        property var colors: [
            "red",
            "orange",
            "yellow",
            "green",
            "blue",
            "violet"
        ]
        property int gridSize: circles.colors.length*5
        model: gridSize*gridSize
        MapCircle
        {
            center
            {
                longitude: -180+360*(index%circles.gridSize+0.5)/(circles.gridSize)
                latitude: -90+180*(Math.floor(index/circles.gridSize)+0.5)/(circles.gridSize)
            }
            radius: 100
            color: circles.colors[Math.floor(index%circles.colors.length)]
            border.width: 2
            autoFadeIn: false
            opacity: 0.1
            NumberAnimation on radius
            {
                loops: Animation.Infinite
                from: 100*1000
                to: (Math.PI*6371-100)*1000
                duration: 10000
            }
        }
    }

    Keys.onPressed: (event)=> {
        Qt.quit()
    }
}
