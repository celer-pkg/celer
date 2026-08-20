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
        id: polylines
        property var colors: [
            "red",
            "orange",
            "yellow",
            "green",
            "blue",
            "violet"
        ]
        property int gridSize: polylines.colors.length*10
        model: gridSize*gridSize
        MapPolygon {
            color: polylines.colors[Math.floor(index%polylines.colors.length)]
            autoFadeIn: false
            property double center_longitude: -180+360*(index%polylines.gridSize+0.5)/(polylines.gridSize)
            property double center_latitude: -90+180*(Math.floor(index/polylines.gridSize)+0.5)/(polylines.gridSize)
            property double a: 1
            geoShape: QtPositioning.polygon(
                [
                    QtPositioning.coordinate(a*center_latitude + 1.11, a*center_longitude + -3.13),
                    QtPositioning.coordinate(a*center_latitude + 0.42, a*center_longitude + 10.54),
                    QtPositioning.coordinate(a*center_latitude + 8.36, a*center_longitude + 6.70)
                ],
                [
                    [
                       QtPositioning.coordinate(a*center_latitude + 4.36, a*center_longitude + 3.46),
                       QtPositioning.coordinate(a*center_latitude + 1.91, a*center_longitude + 0.52),
                       QtPositioning.coordinate(a*center_latitude + 1.50, a*center_longitude + 8.25),
                       QtPositioning.coordinate(a*center_latitude + 4.36, a*center_longitude + 6.80)
                   ]
                ]
            )
            opacity: 0.5
            NumberAnimation on a
            {
                loops: Animation.Infinite
                from: 0
                to: 1
                duration: 10000
            }
        }
    }

    Keys.onPressed: (event)=> {
        Qt.quit()
    }
}

